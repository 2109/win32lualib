#include "dns_resolver.h"

#include "ares.h"
#include "khash.h"

typedef struct dns_task {
	int fd;
	struct event* rio;
	struct event* wio;
} dns_task_t;

KHASH_MAP_INIT_INT(task, dns_task_t*);

typedef khash_t(task) task_hash_t;

typedef struct dns_resolver {
	ares_channel channel;
	struct ares_options opts;
	task_hash_t* hash;
	struct event* timer;
	struct ev_loop_ctx* ev_loop;
} dns_resolver_t;

typedef struct query_param {
	dns_resolve_result cb;
	void* ud;
} query_param_t;

void
task_hash_set(task_hash_t* hash, int fd, dns_task_t* task) {
	int ok;
	khiter_t k = kh_put(task, hash, fd, &ok);
	assert(ok == 1 || ok == 2);
	kh_value(hash, k) = task;
}

dns_task_t*
task_hash_get(task_hash_t* hash, int fd) {
	khiter_t k = kh_get(task, hash, fd);
	if (k == kh_end(hash)) {
		return NULL;
	}
	return kh_value(hash, k);
}

void
task_hash_del(task_hash_t* hash, int fd) {
	khiter_t k = kh_get(task, hash, fd);
	kh_del(task, hash, k);
}

size_t
task_hash_count(task_hash_t* hash) {
	return kh_size(hash);
}

static void
timer_cb(evutil_socket_t fd, short events, void * userdata) {
	dns_resolver_t* resolver = (dns_resolver_t*)userdata;
	ares_process_fd(resolver->channel, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
	struct timeval val;
	val.tv_sec = 0;
	val.tv_usec = 1000 * 100;
	evtimer_add(resolver->timer, &val);
}

static void
dns_poll_cb(evutil_socket_t fd, short events, void * userdata) {
	dns_resolver_t* resolver = (dns_resolver_t*)userdata;

	int w = ARES_SOCKET_BAD, r = ARES_SOCKET_BAD;
	if (events & EV_READ) r = fd;
	if (events & EV_WRITE) w = fd;
	ares_process_fd(resolver->channel, r, w);
}


static void
dns_sock_state_cb(void* ud, ares_socket_t sock, int readable, int writable) {
	dns_resolver_t* resolver = ud;
	dns_task_t* task = task_hash_get(resolver->hash, sock);
	if (readable || writable) {
		if (!task) {
			if (!evtimer_pending(resolver->timer, NULL)) {
				struct timeval val;
				val.tv_sec = 0;
				val.tv_usec = 10000 * 100;
				evtimer_add(resolver->timer, &val);
			}

			task = malloc(sizeof(*task));
			memset(task, 0, sizeof(*task));
			task->fd = sock;
			task->rio = event_new(loop_ctx_get(resolver->ev_loop), sock, EV_READ | EV_PERSIST, dns_poll_cb, resolver);
			task->wio = event_new(loop_ctx_get(resolver->ev_loop), sock, EV_WRITE, dns_poll_cb, resolver);
			task_hash_set(resolver->hash, sock, task);
		}

		if (readable) {
			event_add(task->rio, NULL);
		}

		if (writable) {
			event_add(task->wio, NULL);
		}
	}
	else {
		task_hash_del(resolver->hash, sock);

		if (event_pending(task->rio, EV_READ, NULL)) {
			event_del(task->rio);
		}
		if (event_pending(task->wio, EV_WRITE, NULL)) {
			event_del(task->wio);
		}

		free(task);

		if (task_hash_count(resolver->hash) == 0 && evtimer_pending(resolver->timer, NULL)) {
			evtimer_del(resolver->timer);
		}
	}
}

dns_resolver_t*
dns_resolver_new(struct ev_loop_ctx* ev_loop) {
	ares_library_init(ARES_LIB_INIT_ALL);

	dns_resolver_t* resolver = malloc(sizeof(*resolver));
	memset(resolver, 0, sizeof(*resolver));

	resolver->opts.timeout = 1000;
	resolver->opts.tries = 10;
	resolver->opts.sock_state_cb_data = resolver;
	resolver->opts.sock_state_cb = dns_sock_state_cb;

	if (ares_init_options(&resolver->channel, &resolver->opts, ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES | ARES_OPT_TRIES | ARES_OPT_SOCK_STATE_CB) != ARES_SUCCESS) {
		free(resolver);
		ares_library_cleanup();
		return NULL;
	}
	ares_set_servers_csv(resolver->channel, "114.114.114.114");
	resolver->hash = kh_init(task);

	resolver->ev_loop = ev_loop;
	resolver->timer = evtimer_new(loop_ctx_get(ev_loop), timer_cb, resolver);

	return resolver;
}

void
dns_resolver_delete(dns_resolver_t* resolver) {
	kh_destroy(task, resolver->hash);
	ares_destroy(resolver->channel);
	ares_library_cleanup();
	free(resolver);
}

static void
query_callback(void* ud, int status, int timeouts, struct hostent *host) {
	query_param_t* param = ud;
	if (status != ARES_SUCCESS) {
		param->cb(0, NULL, ares_strerror(status), param->ud);
	}
	else {
		param->cb(1, host, NULL, param->ud);
	}
	free(param);
}

void
dns_query(dns_resolver_t* resolver, const char* name, dns_resolve_result cb, void* ud) {
	query_param_t* param = malloc(sizeof(*param));
	param->cb = cb;
	param->ud = ud;
	ares_gethostbyname(resolver->channel, name, AF_INET, query_callback, param);
}

const char*
dns_last_error(int status) {
	return ares_strerror(status);
}