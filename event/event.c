#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <event2/event.h>  
#include <event2/util.h>
#include <event2/buffer.h>  
#include <event2/buffer_compat.h>  
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/dns.h>

#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/prctl.h> 
#include <sys/un.h>
#endif

#define LUA_EV_ERROR    0
#define LUA_EV_TIMEOUT	1
#define LUA_EV_ACCEPT   2
#define LUA_EV_CONNECT  3
#define LUA_EV_DATA     4
#define LUA_EV_HTTP 	5
#define LUA_EV_DNS		6

#define META_EVENT 			"meta_event"
#define META_EVBUFFER 		"meta_evbuffer"
#define META_TIMER			"meta_timer"
#define META_LISTENER 		"meta_listener"
#define META_HTTP 			"meta_http"
#define META_HTTP_REQUEST 	"meta_http_request"

struct levtimer;

typedef struct levent {
	struct event_base* ev_base;
	struct evdns_base* dns_base;
	lua_State* L;
	int ref;
	int callback;
	struct levtimer* freelist;
} levent_t;

typedef struct levbuffer {
	levent_t* levent;
	struct bufferevent* core;
	int ref;
	int closed;
	int connect_session;
} levbuffer_t;

typedef struct levlistener {
	levent_t* levent;
	struct evconnlistener* listener;
	int ref;
	int closed;
} levlistener_t;

typedef struct levtimer {
	levent_t* levent;
	struct event* ev;
	int ref;
	int cancel;
	struct levtimer* next;
} levtimer_t;

typedef struct lhttpd {
	levent_t* levent;
	struct evhttp* ev;
	int ref;
	int closed;
} lhttpd_t;

typedef struct lrequest {
	struct evhttp_request* request;
	int ref;
	int closed;
} lrequest_t;

typedef struct levdns {
	levent_t* levent;
	struct evdns_getaddrinfo_request* req;
	int ref;
} levdns_t;

static int _bufferevent_destroy(levbuffer_t* levbuffer);
static levbuffer_t* _bufferevent_create(lua_State* L, levent_t* levent, evutil_socket_t sock, int opt);

static int
_meta_init(lua_State* L, const char* meta) {
	luaL_newmetatable(L, meta);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, -1);
	return luaL_ref(L, LUA_REGISTRYINDEX);
}

static void
read_complete(struct bufferevent* core, void* ud) {
	levbuffer_t* levbuffer = ud;
	levent_t* levent = levbuffer->levent;
	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levent->callback);
	lua_pushinteger(levent->L, LUA_EV_DATA);
	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levbuffer->ref);
	lua_pcall(levent->L, 2, 0, 0);
}

static void
write_complete(struct bufferevent* core, void* ud) {
	levbuffer_t* levbuffer = ud;
	levent_t* levent = levbuffer->levent;
	assert(levbuffer->closed == 1);

	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levent->callback);
	lua_pushinteger(levent->L, LUA_EV_ERROR);
	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levbuffer->ref);
	lua_pcall(levent->L, 2, 0, 0);

	_bufferevent_destroy(levbuffer);
}

static void
event_happen(struct bufferevent* core, short events, void* ud) {
	levbuffer_t* levbuffer = ud;
	levent_t* levent = levbuffer->levent;

	if ( events & ( BEV_EVENT_ERROR | BEV_EVENT_EOF ) ) {
		lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levent->callback);
		lua_pushinteger(levent->L, LUA_EV_ERROR);
		lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levbuffer->ref);
		lua_pcall(levent->L, 2, 0, 0);
		levbuffer->closed = 1;
		_bufferevent_destroy(levbuffer);
	}
	else {
		assert(0);
	}
}

static void
connect_complete(struct bufferevent* core, short events, void* ud) {
	levbuffer_t* levbuffer = ud;
	levent_t* levent = levbuffer->levent;

	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levent->callback);
	lua_pushinteger(levent->L, LUA_EV_CONNECT);
	lua_pushinteger(levent->L, levbuffer->connect_session);
	if ( events & BEV_EVENT_CONNECTED ) {
		lua_pushboolean(levent->L, 1);
		lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levbuffer->ref);
		bufferevent_setcb(levbuffer->core, read_complete, NULL, event_happen, levbuffer);
		bufferevent_enable(levbuffer->core, EV_READ);
	}
	else if ( events & BEV_EVENT_ERROR ){
		lua_pushboolean(levent->L, 0);
		evutil_socket_t fd = bufferevent_getfd(core);
		(void)fd;
		int err = evutil_socket_geterror(fd);
		lua_pushstring(levent->L, evutil_socket_error_to_string(err));
		levbuffer->closed = 1;
		_bufferevent_destroy(levbuffer);
	}

	lua_pcall(levent->L, 4, 0, 0);
	return;
}

static void
accept_socket(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr* addr, int socklen, void *ud) {
	levlistener_t* levlistener = ud;
	levent_t* levent = levlistener->levent;

	evutil_make_socket_nonblocking(fd);
	evutil_make_socket_closeonexec(fd);

	levbuffer_t* levbuffer = _bufferevent_create(levent->L, levent, fd, BEV_OPT_CLOSE_ON_FREE);

	bufferevent_setcb(levbuffer->core, read_complete, NULL, event_happen, levbuffer);
	bufferevent_enable(levbuffer->core, EV_READ);

	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levent->callback);
	lua_pushinteger(levent->L, LUA_EV_ACCEPT);
	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levlistener->ref);
	lua_pushvalue(levent->L, -4);

	if ( addr->sa_family == AF_INET ) {
		lua_pushstring(levent->L, inet_ntoa(( ( struct sockaddr_in* )addr )->sin_addr));
		lua_pushinteger(levent->L, ( ( struct sockaddr_in* )addr )->sin_port);
	}
	else if ( addr->sa_family == AF_UNIX ){
		lua_pushstring(levent->L, "unknown");
		lua_pushnil(levent->L);
	}

	lua_pcall(levent->L, 5, 0, 0);
}

static void
timeout(int fd, short event, void* ud) {
	levtimer_t* levtimer = ud;
	levent_t* levent = levtimer->levent;
	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levent->callback);
	lua_pushinteger(levent->L, LUA_EV_TIMEOUT);
	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levtimer->ref);
	lua_pcall(levent->L, 2, 0, 0);
}

static void
dns_response(int err, struct evutil_addrinfo *ai, void* ud) {
	levdns_t* levdns = ud;
	levent_t* levent = levdns->levent;
	
	int args_count = 2;

	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levent->callback);
	lua_pushinteger(levent->L, LUA_EV_DNS);
	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levdns->ref);

	if ( err ) {
		lua_pushboolean(levent->L, 0);
		lua_pushstring(levent->L, evutil_gai_strerror(err));
		args_count += 2;
	}
	else {
		lua_newtable(levent->L);

		int i;
		for ( i = 0; ai; ai = ai->ai_next, i++ ) {
			char buf[128];
			if ( ai->ai_family == PF_INET ) {
				struct sockaddr_in *sin = ( struct sockaddr_in* )ai->ai_addr;
				evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof( buf ));
			}
			else {
				struct sockaddr_in6 *sin6 = ( struct sockaddr_in6* )ai->ai_addr;
				evutil_inet_ntop(AF_INET6, &sin6->sin6_addr, buf, sizeof( buf ));
			}
			lua_pushstring(levent->L, buf);
			lua_rawseti(levent->L, -2, i + 1);
		}

		args_count += 1;

		evutil_freeaddrinfo(ai);
	}

	lua_pcall(levent->L, args_count, 0, 0);
	
	luaL_unref(levent->L, LUA_REGISTRYINDEX, levdns->ref);
}

static void
on_httpd_request(struct evhttp_request *req, void *ud) {
	lhttpd_t* lhttpd = ud;
	levent_t* levent = lhttpd->levent;

	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, levent->callback);
	lua_pushinteger(levent->L, LUA_EV_HTTP);
	lua_rawgeti(levent->L, LUA_REGISTRYINDEX, lhttpd->ref);

	lrequest_t* lrequest = lua_newuserdata(levent->L, sizeof( *lrequest ));
	lrequest->request = req;
	lrequest->closed = 0;
	lrequest->ref = _meta_init(levent->L, META_HTTP_REQUEST);

	switch ( evhttp_request_get_command(req) ) {
		case EVHTTP_REQ_GET: {
			lua_pushstring(levent->L, "GET");
			break;
		}
		case EVHTTP_REQ_POST: {
			lua_pushstring(levent->L, "POST");
			break;
		}
		default: {
			lua_pushstring(levent->L, "UNKNOWN");
			break;
		}
	}

	const struct evhttp_uri * uri = evhttp_request_get_evhttp_uri(req);

	lua_pushstring(levent->L, evhttp_uri_get_path(uri));
	lua_pushstring(levent->L, evhttp_uri_get_query(uri));

	lua_newtable(levent->L);
	struct evkeyvalq *headers;
	struct evkeyval *header;
	headers = evhttp_request_get_input_headers(req);
	for ( header = headers->tqh_first; header; header = header->next.tqe_next ) {
		lua_pushstring(levent->L, header->key);
		lua_pushstring(levent->L, header->value);
		lua_settable(levent->L, -3);
	}

	struct evbuffer *buf = evhttp_request_get_input_buffer(req);
	size_t len = evbuffer_get_length(buf);
	if ( len == 0 ) {
		lua_pushnil(levent->L);
	}
	else {
		char* data = malloc(len);
		evbuffer_remove(buf, data, len);
		lua_pushlstring(levent->L, data, len);
		free(data);
	}

	lua_pcall(levent->L, 8, 0, 0);
}

levbuffer_t*
get_evbuffer(lua_State* L) {
	levbuffer_t* levbuffer = (levbuffer_t*)lua_touserdata(L, 1);
	if ( levbuffer->closed ) {
		luaL_error(L, "evbuff:0x%x already closed", levbuffer);
	}
	return levbuffer;
}

static int
_bufferevent_read(lua_State* L) {
	levbuffer_t* levbuffer = get_evbuffer(L);
	size_t size = luaL_optinteger(L, 2, 0);

	struct evbuffer* input = bufferevent_get_input(levbuffer->core);
	size_t len = evbuffer_get_length(input);
	if (len == 0) {
		return 0;
	}
	if (size == 0 || size > len) {
		size = len;
	}

	char* data = malloc(size);
	evbuffer_remove(input, data, size);
	lua_pushlstring(L, data, size);
	free(data);
	return 1;
}

static int
_bufferevent_read_line(lua_State* L) {
	levbuffer_t* levbuffer = get_evbuffer(L);
	struct evbuffer* input = bufferevent_get_input(levbuffer->core);
	size_t len;
	char* line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF_STRICT);

	if (!line) {
		return 0;
	}

	lua_pushlstring(L, line, len);
	return 1;
}

static int
_bufferevent_write(lua_State* L) {
	levbuffer_t* levbuffer = get_evbuffer(L);
	size_t size;
	const char* data = luaL_checklstring(L, 2, &size);
	if (size == 0) {
		lua_pushboolean(L, 0);
	} else {
		int ok = bufferevent_write(levbuffer->core, data, size);
		lua_pushboolean(L, ok == 0);
	}
	return 1;
}

static int
_bufferevent_destroy(levbuffer_t* levbuffer) {
	levent_t* levent = levbuffer->levent;
	luaL_unref(levent->L, LUA_REGISTRYINDEX, levbuffer->ref);
	bufferevent_free(levbuffer->core);
	return 0;
}

static int
_bufferevent_close(lua_State* L) {
	levbuffer_t* levbuffer = get_evbuffer(L);
	int immediately = luaL_optinteger(L, 2, 1);

	if (levbuffer->closed == 1) {
		luaL_error(L, "evbuffer:0x%x already close", levbuffer);
	}
	levbuffer->closed = 1;

	if (immediately == 1) {
		_bufferevent_destroy(levbuffer);
	} else {
		bufferevent_setcb(levbuffer->core, NULL, write_complete, event_happen, levbuffer);
		bufferevent_enable(levbuffer->core, EV_WRITE);
		bufferevent_disable(levbuffer->core, EV_READ);
	}
	
	return 0;
}

static int
_bufferevent_alive(lua_State* L) {
	levbuffer_t* levbuffer = ( levbuffer_t* )lua_touserdata(L, 1);
	lua_pushboolean(L, levbuffer->closed == 0);
	return 1;
}

static levbuffer_t*
_bufferevent_create(lua_State* L, levent_t* levent, evutil_socket_t sock, int opt) {
	levbuffer_t* levbuffer = lua_newuserdata(L, sizeof( levbuffer_t ));
	memset(levbuffer, 0, sizeof( *levbuffer ));
	levbuffer->levent = levent;
	levbuffer->closed = 0;

	levbuffer->core = bufferevent_socket_new(levent->ev_base, sock, opt);

	levbuffer->ref = _meta_init(L, META_EVBUFFER);

	return levbuffer;
}

static int
_reply_send(lua_State* L) {
	lrequest_t* lrequest = ( lrequest_t* )lua_touserdata(L, 1);
	if ( lrequest->closed == 1 ) {
		luaL_error(L, "request already closed");
	}
	luaL_unref(L, LUA_REGISTRYINDEX, lrequest->ref);

	lrequest->closed = 1;

	int code = lua_tointeger(L, 2);
	const char* reason = lua_tostring(L, 3);
	size_t size;
	const char* data = lua_tolstring(L, 4, &size);

	if ( !data ) {
		evhttp_send_reply(lrequest->request, code, reason, NULL);
	}
	else {
		struct evbuffer *evb = evbuffer_new();
		evbuffer_add(evb, data, size);
		evhttp_send_reply(lrequest->request, code, reason, evb);
		evbuffer_free(evb);
	}

	return 0;
}

static int
_reply_set_header(lua_State* L) {
	lrequest_t* lrequest = ( lrequest_t* )lua_touserdata(L, 1);
	if ( lrequest->closed == 1 ) {
		luaL_error(L, "request already closed");
	}
	const char* key = lua_tostring(L, 2);
	const char* value = lua_tostring(L, 3);

	evhttp_add_header(evhttp_request_get_output_headers(lrequest->request), key, value);
	return 0;
}

static int
_listen_close(lua_State* L) {
	levlistener_t* levlistener = ( levlistener_t* )lua_touserdata(L, 1);
	if ( levlistener->closed ) {
		luaL_error(L, "listener:0x%x alreay closed", levlistener);
	}
	levlistener->closed = 1;
	luaL_unref(L, LUA_REGISTRYINDEX, levlistener->ref);
	evconnlistener_free(levlistener->listener);
	return 0;
}

static int
_listen_alive(lua_State* L) {
	levlistener_t* levlistener = ( levlistener_t* )lua_touserdata(L, 1);
	lua_pushboolean(L, levlistener->closed == 0);
	return 1;
}

union un_sockaddr {
#ifdef _LINUX
	struct sockaddr_un su;
#endif
	struct sockaddr_in si;
};

struct sockaddr*
make_addr(lua_State* L, int index, union un_sockaddr* sa, int* len, int remove) {
	luaL_checktype(L, index, LUA_TTABLE);
	lua_getfield(L, index, "file");

	struct sockaddr* addr;

	if (!lua_isnil(L, -1)) {
#ifdef _LINUX
		sa->su.sun_family = AF_UNIX;  
		const char* file = luaL_checkstring(L, -1);
		strcpy(sa->su.sun_path, file);
		if (remove) {
			unlink(file);
		}
		lua_pop(L, 1);

		addr = (struct sockaddr*)&sa->su;
		*len = sizeof(sa->su);
#else
		luaL_error(L, "no support unix socket");
#endif
	} else {
		sa->si.sin_family = AF_INET;

		lua_pop(L, 1);
		lua_getfield(L, index, "ip");
		const char* ip = luaL_checkstring(L, -1);
		sa->si.sin_addr.s_addr = inet_addr(ip);
		lua_pop(L, 1);

		lua_getfield(L, index, "port");
		int port = luaL_checkinteger(L, -1);
		sa->si.sin_port = htons(port);
		lua_pop(L, 1);

		addr = (struct sockaddr*)&sa->si;
		*len = sizeof( sa->si );
	}
	return addr;
}

static int
_listen(lua_State* L) {
	levent_t* levent = ( levent_t* )lua_touserdata(L, 1);

	int multi = lua_toboolean(L, 2);

	union un_sockaddr addr_un;
	int len;
	struct sockaddr* addr = make_addr(L, 3, &addr_un, &len, 1);

	levlistener_t* levlistener = lua_newuserdata(L, sizeof( *levlistener ));
	levlistener->levent = levent;
	levlistener->closed = 0;

	int flag = LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC;
	if ( multi ) {
		flag |= LEV_OPT_REUSEABLE_PORT;
	}

	struct evconnlistener* listener = evconnlistener_new_bind(levent->ev_base, accept_socket, levlistener, flag, 16, addr, len);
	if ( !listener ) {
		lua_pushboolean(L, 0);
		lua_pushstring(L, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		return 0;
	}

	levlistener->listener = listener;

	levlistener->ref = _meta_init(L, META_LISTENER);

	return 1;
}

static int
_connect(lua_State* L) {
	levent_t* levent = ( levent_t* )lua_touserdata(L, 1);
	int session = lua_tointeger(L, 2);

	union un_sockaddr addr_un;
	int len;
	struct sockaddr* addr = make_addr(L, 3, &addr_un, &len, 0);

	levbuffer_t* levbuffer = _bufferevent_create(L, levent, -1, BEV_OPT_CLOSE_ON_FREE);
	levbuffer->levent = levent;
	levbuffer->connect_session = session;

	bufferevent_setcb(levbuffer->core, NULL, NULL, connect_complete, levbuffer);
	int result = bufferevent_socket_connect(levbuffer->core, addr, len);
	if ( !result ) {
		evutil_socket_t fd = bufferevent_getfd(levbuffer->core);
		evutil_make_socket_closeonexec(fd);
		lua_pushboolean(L, 1);
		return 1;
	}

	evutil_socket_t fd = bufferevent_getfd(levbuffer->core);
	(void)fd;
	int err = evutil_socket_geterror(fd);
	lua_pushstring(L, evutil_socket_error_to_string(err));
	levbuffer->closed = 1;
	_bufferevent_destroy(levbuffer);
	lua_pushboolean(L, 0);
	lua_pushvalue(L, -2);
	return 2;
}

static int
_bind(lua_State* L) {
	levent_t* levent = ( levent_t* )lua_touserdata(L, 1);
	evutil_socket_t fd = (evutil_socket_t)lua_tointeger(L, 2);

	levbuffer_t* levbuffer = _bufferevent_create(L, levent, fd, 0);
	bufferevent_setcb(levbuffer->core, read_complete, NULL, event_happen, levbuffer);
	bufferevent_enable(levbuffer->core, EV_READ);
	return 1;
}

static int
_timer_cancel(lua_State* L) {
	levtimer_t* levtimer = ( levtimer_t* )lua_touserdata(L, 1);
	if ( levtimer->cancel == 1 ) {
		luaL_error(L, "timer:0x%x already cancel", levtimer);
	}
	levent_t* levent = levtimer->levent;
	evtimer_del(levtimer->ev);
	levtimer->cancel = 1;
	levtimer->next = levent->freelist;
	levent->freelist = levtimer;
	return 0;
}

static int
_timer_alive(lua_State* L) {
	levtimer_t* levtimer = ( levtimer_t* )lua_touserdata(L, 1);
	lua_pushboolean(L, levtimer->cancel == 0);
	return 1;
}

static int
_timer(lua_State* L) {
	levent_t* levent = ( levent_t* )lua_touserdata(L, 1);
	struct event_base* ev_base = levent->ev_base;

	double ti = luaL_checknumber(L, 2);
	int once = lua_toboolean(L, 3);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = (long)( ti * 1000000 );
	if ( tv.tv_usec >= 1000000 ) {
		tv.tv_sec = tv.tv_usec / 1000000;
		tv.tv_usec = tv.tv_usec - tv.tv_sec * 1000000;
	}

	int flag = EV_TIMEOUT;
	if ( !once ) {
		flag |= EV_PERSIST;
	}

	levtimer_t* levtimer = NULL;
	if ( levent->freelist ) {
		levtimer = levent->freelist;
		levent->freelist = levent->freelist->next;
		event_assign(levtimer->ev, ev_base, -1, flag, timeout, levtimer);
		lua_rawgeti(L, LUA_REGISTRYINDEX, levtimer->ref);
	}
	else {
		levtimer = lua_newuserdata(L, sizeof( *levtimer ));
		levtimer->levent = levent;
		levtimer->ref = _meta_init(L, META_TIMER);
		levtimer->ev = event_new(ev_base, -1, flag, timeout, levtimer);
	}

	levtimer->cancel = 0;

	evtimer_add(levtimer->ev, &tv);

	return 1;
}

static int
_dns(lua_State* L) {
	levent_t* levent = ( levent_t* )lua_touserdata(L, 1);
	const char* host = lua_tostring(L, 2);

	struct evutil_addrinfo hints;
	memset(&hints, 0, sizeof( hints ));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = EVUTIL_AI_CANONNAME;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	levdns_t* levdns = lua_newuserdata(L, sizeof( *levdns ));
	levdns->levent = levent;

	lua_pushvalue(L, -1);
	levdns->ref = luaL_ref(L, LUA_REGISTRYINDEX);

	levdns->req = evdns_getaddrinfo(levent->dns_base, host, NULL, &hints, dns_response, levdns);

	if ( !levdns->req ) {
		lua_pushboolean(L, 0);
	}
	
	return 1;
}

static int
_httpd_close(lua_State* L) {
	lhttpd_t* lhttpd = (lhttpd_t*)lua_touserdata(L, 1);
	if ( lhttpd->closed == 1 ) {
		luaL_error(L, "httpd already closed");
	}
	luaL_unref(L, LUA_REGISTRYINDEX, lhttpd->ref);
	evhttp_free(lhttpd->ev);
	lhttpd->closed = 1;
	return 0;
}

static int
_httpd_alive(lua_State* L) {
	lhttpd_t* lhttpd = (lhttpd_t*)lua_touserdata(L, 1);
	lua_pushboolean(L, lhttpd->closed == 0);
	return 1;
}

static int
_httpd(lua_State* L) {
	levent_t* levent = (levent_t*)lua_touserdata(L, 1);
	size_t sz;
	const char* ip = lua_tolstring(L, 2, &sz);
	int port = lua_tointeger(L, 3);
	int multi = lua_toboolean(L, 4);

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(ip);
	sin.sin_port = htons(port);
	memset(sin.sin_zero, 0x00, 8);

	int flag = LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC;
	if ( multi ) {
		flag |= LEV_OPT_REUSEABLE_PORT;
	}

	struct evconnlistener* listener = evconnlistener_new_bind(levent->ev_base, NULL, NULL, flag, 16, ( struct sockaddr* )&sin, sizeof( struct sockaddr_in ));
	if ( !listener ) {
		return 0;
	}
	struct evhttp* ev = evhttp_new(levent->ev_base);
	if ( !evhttp_bind_listener(ev, listener) ) {
		evconnlistener_free(listener);
		evhttp_free(ev);
		return 0;
	}

	lhttpd_t* lhttpd = lua_newuserdata(L, sizeof( *lhttpd ));
	lhttpd->levent = levent;
	lhttpd->ev = ev;
	lhttpd->closed = 0;
	lhttpd->ref = _meta_init(L, META_HTTP);

	evhttp_set_gencb(lhttpd->ev, on_httpd_request, lhttpd);

	return 1;
}

static int
_dispatch(lua_State* L) {
	levent_t* levent = ( levent_t* )lua_touserdata(L, 1);
	struct event_base* ev_base = levent->ev_base;
	int result = event_base_dispatch(ev_base);
	lua_pushinteger(L, result);
	return 1;
}

static int
_release(lua_State* L) {
	levent_t* levent = ( levent_t* )lua_touserdata(L, 1);
	while ( levent->freelist ) {
		levtimer_t* timer = levent->freelist;
		levent->freelist = timer->next;
		event_free(timer->ev);
	}
	evdns_base_free(levent->dns_base, 1);
	event_base_free(levent->ev_base);
	luaL_unref(L, LUA_REGISTRYINDEX, levent->ref);
	return 0;
}

static int
_break(lua_State* L) {
	levent_t* levent = ( levent_t* )lua_touserdata(L, 1);
	struct event_base* ev_base = levent->ev_base;
	int result = event_base_loopbreak(ev_base);
	lua_pushinteger(L, result);
	return 1;
}

static int
_now(lua_State* L) {
	struct timeval tv;
	evutil_gettimeofday(&tv, NULL);
	uint64_t now = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
	lua_pushinteger(L, now);
	return 1;
}

static int
_sleepex(lua_State* L) {
	levent_t* levent = ( levent_t* )lua_touserdata(L, 1);
	int ti = lua_tointeger(L, 2);
	Sleep(ti);
	return 0;
}

static int
_event_new(lua_State* L) {
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	int callback = luaL_ref(L, LUA_REGISTRYINDEX);
	levent_t* levent = lua_newuserdata(L, sizeof( *levent ));
	levent->ev_base = event_base_new();
	levent->dns_base = evdns_base_new(levent->ev_base, EVDNS_BASE_INITIALIZE_NAMESERVERS);
	levent->L = L;
	levent->callback = callback;
	levent->freelist = NULL;
	levent->ref = _meta_init(L, META_EVENT);

	return 1;
}

EXPORT int
luaopen_event_core(lua_State* L) {
	luaL_newmetatable(L, META_EVENT);
	const luaL_Reg meta_event[] = {
		{ "listen", _listen },
		{ "connect", _connect },
		{ "timer", _timer },
		{ "bind", _bind },
		{ "httpd", _httpd },
		{ "dns", _dns },
		{ "breakout", _break },
		{ "dispatch", _dispatch },
		{ "release", _release },
		{ "now", _now },
		{ "sleep", _sleepex },
		{ NULL, NULL },
	};
	luaL_newlib(L, meta_event);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	luaL_newmetatable(L, META_EVBUFFER);
	const luaL_Reg meta_buffer[] = {
		{ "write", _bufferevent_write },
		{ "read", _bufferevent_read },
		{ "read_line", _bufferevent_read_line },
		{ "alive", _bufferevent_alive },
		{ "close", _bufferevent_close },
		{ NULL, NULL },
	};
	luaL_newlib(L, meta_buffer);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	luaL_newmetatable(L, META_TIMER);
	const luaL_Reg meta_timer[] = {
		{ "cancel", _timer_cancel },
		{ "alive", _timer_alive },
		{ NULL, NULL },
	};
	luaL_newlib(L, meta_timer);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	luaL_newmetatable(L, META_LISTENER);
	const luaL_Reg meta_listener[] = {
		{ "close", _listen_close },
		{ "alive", _listen_alive },
		{ NULL, NULL },
	};
	luaL_newlib(L, meta_listener);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	luaL_newmetatable(L, META_HTTP_REQUEST);
	const luaL_Reg meta_request[] = {
		{ "reply", _reply_send },
		{ "set_header", _reply_set_header },
		{ NULL, NULL },
	};
	luaL_newlib(L, meta_request);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	luaL_newmetatable(L, META_HTTP);
	const luaL_Reg meta_http[] = {
		{ "close", _httpd_close },
		{ "alive", _httpd_alive },
		{ NULL, NULL },
	};
	luaL_newlib(L, meta_http);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	const luaL_Reg l[] = {
		{ "new", _event_new },
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	return 1;
}
