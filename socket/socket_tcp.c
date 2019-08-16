
#include "socket_tcp.h"
#include <io.h>
#include <windows.h>
#define MIN_BUFFER_SIZE 256
#define MAX_BUFFER_SIZE 1024*1024

#define inline __inline

typedef struct data_buffer {
	struct data_buffer* prev;
	struct data_buffer* next;
	void* data;
	int size;
	int wpos;
	int rpos;
} data_buffer_t;

typedef struct ev_buffer {
	data_buffer_t* head;
	data_buffer_t* tail;
	int total;
} ev_buffer_t;

typedef struct ev_loop_ctx {
	struct event_base* loop;
	data_buffer_t* freelist;
} ev_loop_ctx_t;

typedef struct ev_listener {
	struct ev_loop_ctx* loop_ctx;
	struct event* rio;
	int fd;
	listener_callback accept_cb;
	void* userdata;
} ev_listener_t;

typedef struct ev_session {
	struct ev_loop_ctx* loop_ctx;
	struct event* rio;
	struct event* wio;

	int fd;
	int alive;
	int max;
	int threshold;

	ev_buffer_t input;
	ev_buffer_t output;

	ev_session_callback read_cb;
	ev_session_callback write_cb;
	ev_session_callback event_cb;

	void* userdata;
} ev_session_t;

void ev_session_free(ev_session_t* ev_session);
void ev_session_disable(ev_session_t* ev_session, int ev);

static inline data_buffer_t*
buffer_next(ev_loop_ctx_t* loop_ctx) {
	data_buffer_t* db = NULL;
	if (loop_ctx->freelist != NULL) {
		db = loop_ctx->freelist;
		loop_ctx->freelist = db->next;
	}
	else {
		db = malloc(sizeof(*db));
	}
	db->data = NULL;
	db->wpos = db->rpos = 0;
	db->size = 0;
	db->prev = NULL;
	db->next = NULL;

	return db;
}

static inline void
buffer_reclaim(ev_loop_ctx_t* loop_ctx, data_buffer_t* db) {
	db->next = loop_ctx->freelist;
	loop_ctx->freelist = db;
}

static inline void
buffer_append(ev_buffer_t* ev_buffer, data_buffer_t* db) {
	ev_buffer->total += db->wpos - db->rpos;
	if (ev_buffer->head == NULL) {
		assert(ev_buffer->tail == NULL);
		ev_buffer->head = ev_buffer->tail = db;
	}
	else {
		ev_buffer->tail->next = db;
		db->prev = ev_buffer->tail;
		db->next = NULL;
		ev_buffer->tail = db;
	}
}

static inline void
buffer_release(ev_buffer_t* ev_buffer) {
	while (ev_buffer->head) {
		data_buffer_t* tmp = ev_buffer->head;
		ev_buffer->head = ev_buffer->head->next;
		free(tmp->data);
		free(tmp);
	}
}

static void
_ev_accept_cb(evutil_socket_t fd, short events, void * userdata) {
	ev_listener_t* listener = userdata;

	const char addr[HOST_SIZE] = { 0 };
	int accept_fd = socket_accept(listener->fd, (char*)addr, HOST_SIZE);
	if (accept_fd < 0) {
		fprintf(stderr, "accept fd error:%s\n", addr);
		return;
	}
	listener->accept_cb(listener, accept_fd, addr, listener->userdata);
}

static void
_ev_read_cb(evutil_socket_t fd, short events, void * userdata) {
	ev_session_t* ev_session = userdata;

	data_buffer_t* rdb = buffer_next(ev_session->loop_ctx);
	rdb->data = malloc(ev_session->threshold);
	rdb->size = ev_session->threshold;
	int fail = 0;
	int total = ev_session->input.total;
	for (;;) {
		int n = (int)_read(ev_session->fd, (char*)rdb->data + rdb->wpos, rdb->size - rdb->wpos);
		if (n < 0) {
			if (errno) {
				if (errno == EINTR) {
					continue;
				}
				else if (errno == EAGAIN) {
					break;
				}
				else {
					fail = 1;
					break;
				}
			}
			else {
				assert(0);
			}
		}
		else if (n == 0) {
			fail = 1;
			break;
		}
		else {
			total += n;
			rdb->wpos += n;

			if (rdb->wpos == rdb->size) {
				ev_session->threshold *= 2;
				if (ev_session->threshold > MAX_BUFFER_SIZE)
					ev_session->threshold = MAX_BUFFER_SIZE;
			}
			else {
				ev_session->threshold /= 2;
				if (ev_session->threshold < MIN_BUFFER_SIZE)
					ev_session->threshold = MIN_BUFFER_SIZE;
			}

			if (ev_session->max > 0 && total >= ev_session->max) {
				break;
			}
		}
	}

	buffer_append(&ev_session->input, rdb);

	if (fail) {
		ev_session_disable(ev_session, EV_READ | EV_WRITE);
		ev_session->alive = 0;
		if (ev_session->event_cb) {
			ev_session->event_cb(ev_session, ev_session->userdata);
		}
	}
	else {
		if (ev_session->read_cb) {
			ev_session->read_cb(ev_session, ev_session->userdata);
		}
	}
}

static void
_ev_write_cb(evutil_socket_t fd, short events, void * userdata) {
	ev_session_t* ev_session = userdata;

	while (ev_session->output.head != NULL) {
		data_buffer_t* wdb = ev_session->output.head;
		int left = wdb->wpos - wdb->rpos;
		int total = socket_write(ev_session->fd, (char*)wdb->data + wdb->rpos, left);
		if (total < 0) {
			ev_session_disable(ev_session, EV_READ | EV_WRITE);
			ev_session->alive = 0;
			if (ev_session->event_cb)
				ev_session->event_cb(ev_session, ev_session->userdata);
			return;
		}
		else {
			ev_session->output.total -= total;
			if (total == left) {
				free(wdb->data);
				ev_session->output.head = wdb->next;
				buffer_reclaim(ev_session->loop_ctx, wdb);
				if (ev_session->output.head == NULL) {
					ev_session->output.head = ev_session->output.tail = NULL;
					break;
				}
			}
			else {
				wdb->rpos += total;
				return;
			}
		}
	}

	ev_session_disable(ev_session, EV_WRITE);
	assert(ev_session->output.total == 0);
	if (ev_session->write_cb) {
		ev_session->write_cb(ev_session, ev_session->userdata);
	}
}

ev_loop_ctx_t*
loop_ctx_create() {
	ev_loop_ctx_t* loop_ctx = malloc(sizeof(*loop_ctx));
	memset(loop_ctx, 0, sizeof(*loop_ctx));
	loop_ctx->loop = event_base_new();
	return loop_ctx;
}

void
loop_ctx_release(ev_loop_ctx_t* loop_ctx) {
	event_base_free(loop_ctx->loop);

	while (loop_ctx->freelist) {
		data_buffer_t* tmp = loop_ctx->freelist;
		loop_ctx->freelist = loop_ctx->freelist->next;
		free(tmp);
	}
	free(loop_ctx);
}

struct event_base*
	loop_ctx_get(ev_loop_ctx_t* loop_ctx) {
	return loop_ctx->loop;
}

void
loop_ctx_dispatch(ev_loop_ctx_t* loop_ctx) {
	event_base_dispatch(loop_ctx->loop);
}

void
loop_ctx_break(ev_loop_ctx_t* loop_ctx) {
	event_base_loopbreak(loop_ctx->loop);
}

double
loop_ctx_now(ev_loop_ctx_t* loop_ctx) {
	struct timeval val;
	int status = event_base_gettimeofday_cached(loop_ctx->loop, &val);
	if (status < 0) {
		return -1;
	}
	return val.tv_sec + val.tv_usec / 1000000;
}

void
loop_ctx_clean(ev_loop_ctx_t* loop_ctx) {
	while (loop_ctx->freelist) {
		data_buffer_t* tmp = loop_ctx->freelist;
		loop_ctx->freelist = loop_ctx->freelist->next;
		free(tmp);
	}
}

ev_listener_t*
ev_listener_bind(struct ev_loop_ctx* loop_ctx, struct sockaddr* addr, int addrlen, int backlog, int flag, listener_callback accept_cb, void* userdata) {
	int fd = socket_listen(addr, addrlen, backlog, flag);
	if (fd < 0) {
		return NULL;
	}
	ev_listener_t* listener = malloc(sizeof(*listener));
	listener->loop_ctx = loop_ctx;
	listener->fd = fd;
	listener->accept_cb = accept_cb;
	listener->userdata = userdata;
	listener->rio = event_new(loop_ctx->loop, fd, EV_READ, _ev_accept_cb, listener);
	event_add(listener->rio, NULL);
	return listener;
}

ev_listener_t*
ev_listener_bind_ipv4(struct ev_loop_ctx* loop_ctx, const char* ip, uint16_t port, listener_callback accept_cb, void* userdata) {
	struct sockaddr_in si;
	si.sin_family = AF_INET;
	si.sin_addr.s_addr = inet_addr(ip);
	si.sin_port = htons(port);
	return ev_listener_bind(loop_ctx, (struct sockaddr*)&si, sizeof(si), 16, SOCKET_OPT_NOBLOCK | SOCKET_OPT_REUSEABLE_ADDR, accept_cb, userdata);
}

int
ev_listener_fd(ev_listener_t* listener) {
	return listener->fd;
}

int
ev_listener_addr(ev_listener_t* listener, char* addr, size_t length, int* port) {
	if (get_sockname(listener->fd, addr, length, port) < 0) {
		return -1;
	}
	return 0;
}

void
ev_listener_free(ev_listener_t* listener) {
	if (event_pending(listener->rio, EV_READ, NULL)) {
		event_del(listener->rio);
	}
	event_free(listener->rio);
	closesocket(listener->fd);
	free(listener);
}

ev_session_t*
ev_session_bind(struct ev_loop_ctx* loop_ctx, int fd, int max) {
	ev_session_t* ev_session = malloc(sizeof(*ev_session));
	memset(ev_session, 0, sizeof(*ev_session));
	ev_session->loop_ctx = loop_ctx;
	ev_session->fd = fd;
	ev_session->max = max;
	ev_session->threshold = MIN_BUFFER_SIZE;

	ev_session->rio = event_new(loop_ctx->loop, fd, EV_READ, _ev_read_cb, ev_session);
	ev_session->wio = event_new(loop_ctx->loop, fd, EV_WRITE, _ev_write_cb, ev_session);

	ev_session->alive = 1;

	return ev_session;
}

ev_session_t*
ev_session_connect(struct ev_loop_ctx* loop_ctx, struct sockaddr* addr, int addrlen, int block, int max, int* status) {
	int result = 0;
	int fd = socket_connect(addr, addrlen, block, &result);
	if (fd < 0) {
		*status = CONNECT_STATUS_CONNECT_FAIL;
		return NULL;
	}
	ev_session_t* ev_session = ev_session_bind(loop_ctx, fd, max);

	//if connect op is block,result here must be true
	*status = CONNECT_STATUS_CONNECTING;
	if (result)
		*status = CONNECT_STATUS_CONNECTED;

	return ev_session;
}

void
ev_session_free(ev_session_t* ev_session) {
	ev_session->alive = 0;
	ev_session_disable(ev_session, EV_READ | EV_WRITE);
	event_free(ev_session->rio);
	event_free(ev_session->wio);
	closesocket(ev_session->fd);

	buffer_release(&ev_session->input);
	buffer_release(&ev_session->output);

	free(ev_session);
}

void
ev_session_setcb(ev_session_t* ev_session, ev_session_callback read_cb, ev_session_callback write_cb, ev_session_callback event_cb, void* userdata) {
	ev_session->read_cb = read_cb;
	ev_session->write_cb = write_cb;
	ev_session->event_cb = event_cb;
	ev_session->userdata = userdata;
}

void
ev_session_enable(ev_session_t* ev_session, int ev) {
	if (ev & EV_READ) {
		if (!event_pending(ev_session->rio, EV_READ, NULL)) {
			event_add(ev_session->rio, NULL);
		}
	}
	if (ev & EV_WRITE) {
		if (!event_pending(ev_session->wio, EV_WRITE, NULL)) {
			event_add(ev_session->wio, NULL);
		}
	}
}

void
ev_session_disable(ev_session_t* ev_session, int ev) {
	if (ev & EV_READ) {
		if (event_pending(ev_session->rio, EV_READ, NULL)) {
			event_del(ev_session->rio);
		}
	}
	if (ev & EV_WRITE) {
		if (event_pending(ev_session->wio, EV_WRITE, NULL)) {
			event_del(ev_session->wio);
		}
	}
}

int
ev_session_fd(ev_session_t* ev_session) {
	return ev_session->fd;
}

size_t
ev_session_input_size(ev_session_t* ev_session) {
	return ev_session->input.total;
}

size_t
ev_session_output_size(ev_session_t* ev_session) {
	return ev_session->output.total;
}

static inline int
check_eol(data_buffer_t* db, int from, const char* sep, size_t sep_len) {
	while (db) {
		int sz = db->wpos - from;
		if (sz >= (int)sep_len) {
			return memcmp((char*)db->data + from, sep, sep_len) == 0;
		}

		if (sz > 0) {
			if (memcmp((char*)db->data + from, sep, sz)) {
				return 0;
			}
		}
		db = db->next;
		sep += sz;
		sep_len -= sz;
		from = 0;
	}
	return 0;
}

static inline int
search_eol(ev_session_t* ev_session, const char* sep, size_t sep_len) {
	data_buffer_t* current = ev_session->input.head;
	int offset = 0;
	while (current) {
		int i = current->rpos;
		for (; i < current->wpos; i++) {
			int ret = check_eol(current, i, sep, sep_len);
			if (ret == 1) {
				return offset + sep_len;
			}
			++offset;
		}
		current = current->next;
	}

	return -1;
}

size_t
ev_session_read(struct ev_session* ev_session, char* result, size_t size) {
	if ((int)size > ev_session->input.total)
		size = ev_session->input.total;

	int offset = 0;
	int need = size;
	while (need > 0) {
		data_buffer_t* rdb = ev_session->input.head;
		if (rdb->rpos + need < rdb->wpos) {
			memcpy(result + offset, (char*)rdb->data + rdb->rpos, need);
			rdb->rpos += need;

			offset += need;
			assert(offset == size);
			need = 0;
		}
		else {
			int left = rdb->wpos - rdb->rpos;
			memcpy(result + offset, (char*)rdb->data + rdb->rpos, left);
			offset += left;
			need -= left;
			free(rdb->data);

			data_buffer_t* tmp = ev_session->input.head;

			ev_session->input.head = ev_session->input.head->next;
			if (ev_session->input.head == NULL) {
				ev_session->input.head = ev_session->input.tail = NULL;
				assert(need == 0);
			}

			buffer_reclaim(ev_session->loop_ctx, tmp);
		}
	}
	ev_session->input.total -= size;

	return size;
}

char* ev_session_read_util(ev_session_t* ev_session, const char* sep, size_t size, char* out, size_t out_size, size_t* length) {
	int offset = search_eol(ev_session, sep, size);
	if (offset < 0) {
		return NULL;
	}
	*length = offset;
	char* result = out;
	if (offset > (int)out_size) {
		result = malloc(offset);
	}
	ev_session_read(ev_session, result, offset);
	return result;
}

int
ev_session_write(ev_session_t* ev_session, char* data, size_t size) {
	if (ev_session->alive == 0)
		return -1;

	if (!event_pending(ev_session->wio, EV_WRITE, NULL)) {
		int total = socket_write(ev_session->fd, data, size);
		if (total < 0) {
			ev_session_disable(ev_session, EV_READ | EV_WRITE);
			ev_session->alive = 0;
			if (ev_session->event_cb)
				ev_session->event_cb(ev_session, ev_session->userdata);
			return -1;
		}
		else {
			if (total == size) {
				free(data);
				if (ev_session->write_cb) {
					ev_session->write_cb(ev_session, ev_session->userdata);
				}
			}
			else {
				data_buffer_t* wdb = buffer_next(ev_session->loop_ctx);
				wdb->data = data;
				wdb->rpos = total;
				wdb->wpos = size;
				wdb->size = size;
				buffer_append(&ev_session->output, wdb);
				event_add(ev_session->wio, NULL);
			}
			return total;
		}
	}
	else {
		data_buffer_t* wdb = buffer_next(ev_session->loop_ctx);
		wdb->data = data;
		wdb->rpos = 0;
		wdb->wpos = size;
		wdb->size = size;
		buffer_append(&ev_session->output, wdb);
		return 0;
	}
}
