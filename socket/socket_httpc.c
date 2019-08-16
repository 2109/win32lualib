#include "socket_httpc.h"
#include <event2/event.h>  
#include "curl/curl.h"
#include "string.h"
#include <assert.h>


typedef struct http_multi {
	CURLM* ctx;
	int still_running;
	struct event* timer;
	struct ev_loop_ctx* ev_loop;
} http_multi_t;

typedef struct http_request {
	http_multi_t* multi;
	CURL* ctx;
	int fd;
	struct event* rio;
	struct event* wio;
	void* headers;
	void* content;
	string_t receive_header;
	string_t receive_content;
	char error[CURL_ERROR_SIZE];
	void* callback_ud;
	request_callback callback;
} http_request_t;

static void
check_multi_info(http_multi_t *multi) {
	CURLMsg *msg;
	int msgs_left;
	CURL *easy;
	http_request_t * request = NULL;

	while ((msg = curl_multi_info_read(multi->ctx, &msgs_left))) {
		if (msg->msg == CURLMSG_DONE) {
			easy = msg->easy_handle;
			curl_easy_getinfo(easy, CURLINFO_PRIVATE, &request);
			curl_multi_remove_handle(multi->ctx, easy);
			request->callback(request, get_http_code(request), request->callback_ud);
			http_request_delete(request);
		}
	}
}

static void
timeout_cb(evutil_socket_t fd, short events, void * userdata) {
	http_multi_t* multi = userdata;
	curl_multi_socket_action(multi->ctx, CURL_SOCKET_TIMEOUT, 0, &multi->still_running);
	check_multi_info(multi);
}

static void
read_cb(evutil_socket_t fd, short events, void * userdata) {
	http_request_t* request = userdata;
	http_multi_t* multi = request->multi;
	curl_multi_socket_action(multi->ctx, request->fd, CURL_POLL_IN, &multi->still_running);
	check_multi_info(multi);
	if (multi->still_running <= 0) {
		if (evtimer_pending(multi->timer, NULL)) {
			evtimer_del(multi->timer);
		}
	}
}

static void
write_cb(evutil_socket_t fd, short events, void * userdata) {
	http_request_t* request = userdata;
	event_del(request->wio);
	http_multi_t* multi = request->multi;
	curl_multi_socket_action(multi->ctx, request->fd, CURL_POLL_OUT, &multi->still_running);
	check_multi_info(multi);
	if (multi->still_running <= 0) {
		if (evtimer_pending(multi->timer, NULL)) {
			evtimer_del(multi->timer);
		}
	}
}

static int
multi_sock_cb(CURL* e, curl_socket_t s, int what, void* cbp, void* ud) {
	http_multi_t* multi = cbp;
	http_request_t* request = ud;
	if (what == CURL_POLL_REMOVE) {
		if (event_pending(request->rio, EV_READ, NULL)) {
			event_del(request->rio);
		}
		if (event_pending(request->wio, EV_WRITE, NULL)) {
			event_del(request->wio);
		}
	}
	else {
		if (!request) {
			curl_easy_getinfo(e, CURLINFO_PRIVATE, &request);
			curl_multi_assign(request->multi->ctx, s, request);
			request->fd = s;
			request->rio = event_new(loop_ctx_get(multi->ev_loop), s, EV_READ, read_cb, request);
			request->wio = event_new(loop_ctx_get(multi->ev_loop), s, EV_WRITE, write_cb, request);
		}

		if (what == CURL_POLL_IN) {
			if (!event_pending(request->rio, EV_READ, NULL)) {
				event_add(request->rio, NULL);
			}
		}
		else if (what == CURL_POLL_OUT){
			if (!event_pending(request->wio, EV_WRITE, NULL)) {
				event_add(request->wio, NULL);
			}
		}
		else if (what == CURL_POLL_INOUT){
			if (!event_pending(request->rio, EV_READ, NULL)) {
				event_add(request->rio, NULL);
			}
			if (!event_pending(request->wio, EV_WRITE, NULL)) {
				event_add(request->wio, NULL);
			}
		}
	}
	return 0;
}

static int
multi_timer_cb(CURLM* ctx, long timeout_ms, void* ud) {
	http_multi_t* multi = ud;

	if (timeout_ms == 0) {
		curl_multi_socket_action(multi->ctx, CURL_SOCKET_TIMEOUT, 0, &multi->still_running);
	}
	else if (timeout_ms > 0) {
		if (evtimer_pending(multi->timer, NULL)) {
			evtimer_del(multi->timer);
		}

		evtimer_assign(multi->timer, loop_ctx_get(multi->ev_loop), timeout_cb, multi);
		struct timeval val;
		val.tv_sec = timeout_ms / 1000;
		val.tv_usec = (double)(timeout_ms % 1000) / 1000000;
		evtimer_add(multi->timer, &val);
	}

	return 0;
}


void
http_multi_init(http_multi_t* multi, struct ev_loop_ctx* ev_loop) {
	assert(curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK);
	multi->ctx = curl_multi_init();
	multi->ev_loop = ev_loop;
	multi->still_running = 0;
	multi->timer = evtimer_new(loop_ctx_get(multi->ev_loop), timeout_cb, multi);
	curl_multi_setopt(multi->ctx, CURLMOPT_SOCKETFUNCTION, multi_sock_cb);
	curl_multi_setopt(multi->ctx, CURLMOPT_SOCKETDATA, multi);
	curl_multi_setopt(multi->ctx, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
	curl_multi_setopt(multi->ctx, CURLMOPT_TIMERDATA, multi);
}

http_multi_t*
http_multi_new(struct ev_loop_ctx* ev_loop) {
	http_multi_t* multi = malloc(sizeof(*multi));
	memset(multi, 0, sizeof(*multi));
	http_multi_init(multi, ev_loop);
	return multi;
}

void
http_multi_release(http_multi_t* multi) {
	event_free(multi->timer);
	curl_global_cleanup();
	curl_multi_cleanup(multi->ctx);
}

void
http_multi_delete(http_multi_t* multi) {
	http_multi_release(multi);
	free(multi);
}

size_t
receive_data(char *buffer, size_t size, size_t nitems, void *userdata) {
	string_t* data = (string_t*)userdata;
	string_append_lstr(data, buffer, nitems * size);
	return nitems * size;
}

void
http_request_init(http_request_t* request) {
	request->error[0] = '\0';
	request->headers = NULL;
	request->content = NULL;
	string_init(&request->receive_header, NULL, 64);
	string_init(&request->receive_content, NULL, 64);
	request->multi = NULL;
	request->callback = NULL;
	request->ctx = curl_easy_init();

	curl_easy_setopt(request->ctx, CURLOPT_PRIVATE, request);

	curl_easy_setopt(request->ctx, CURLOPT_HEADERFUNCTION, receive_data);
	curl_easy_setopt(request->ctx, CURLOPT_HEADERDATA, &request->receive_header);

	curl_easy_setopt(request->ctx, CURLOPT_WRITEFUNCTION, receive_data);
	curl_easy_setopt(request->ctx, CURLOPT_WRITEDATA, &request->receive_content);

	curl_easy_setopt(request->ctx, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(request->ctx, CURLOPT_NOSIGNAL, 1);

	curl_easy_setopt(request->ctx, CURLOPT_CONNECTTIMEOUT_MS, 0);

	curl_easy_setopt(request->ctx, CURLOPT_ERRORBUFFER, request->error);

	curl_easy_setopt(request->ctx, CURLOPT_LOW_SPEED_TIME, 5L);
	curl_easy_setopt(request->ctx, CURLOPT_LOW_SPEED_LIMIT, 30L);
}

void
http_request_release(http_request_t* request) {
	curl_easy_cleanup(request->ctx);
	event_free(request->rio);
	event_free(request->wio);
	if (request->headers) {
		curl_slist_free_all(request->headers);
	}
	if (request->content) {
		free(request->content);
	}
	string_release(&request->receive_header);
	string_release(&request->receive_content);
}

http_request_t*
http_request_new() {
	http_request_t* request = malloc(sizeof(*request));
	memset(request, 0, sizeof(*request));
	http_request_init(request);
	return request;
}

void
http_request_delete(http_request_t* request) {
	http_request_release(request);
	free(request);
}

int
http_request_perform(http_multi_t* multi, http_request_t* request, request_callback callback, void* ud) {
	request->multi = multi;
	request->callback = callback;
	request->callback_ud = ud;
	CURLMcode rc = curl_multi_add_handle(multi->ctx, request->ctx);

	if (rc != CURLM_OK) {
		http_request_delete(request);
		return -1;
	}

	return 0;
}

int
set_url(http_request_t* request, const char* url) {
	if (CURLE_OK != curl_easy_setopt(request->ctx, CURLOPT_URL, url)) {
		return -1;
	}
	return 0;
}

int
set_unix_socket_path(http_request_t* request, const char* path) {
	if (CURLE_OK != curl_easy_setopt(request->ctx, CURLOPT_UNIX_SOCKET_PATH, path)) {
		return -1;
	}
	return 0;
}

int
set_post_data(http_request_t* request, const char* data, size_t size) {
	if (!data) {
		return -1;
	}

	CURLcode status = CURLE_OK;

	status = curl_easy_setopt(request->ctx, CURLOPT_POST, 1);
	if (CURLE_OK != status) {
		return status;
	}

	if (size == 0) {
		status = curl_easy_setopt(request->ctx, CURLOPT_POSTFIELDS, "");
	}
	else {
		request->content = malloc(size);
		memcpy(request->content, data, size);
		status = curl_easy_setopt(request->ctx, CURLOPT_POSTFIELDS, request->content);
	}

	if (CURLE_OK != status) {
		return status;
	}

	status = curl_easy_setopt(request->ctx, CURLOPT_POSTFIELDSIZE, size);
	if (CURLE_OK != status) {
		return status;
	}

	return 0;
}

int
set_header(http_request_t* request, const char* data, size_t size) {
	if (request->headers) {
		curl_slist_append(request->headers, data);
	}
	else {
		request->headers = curl_slist_append(NULL, data);
	}

	CURLcode status = curl_easy_setopt(request->ctx, CURLOPT_HTTPHEADER, request->headers);

	if (CURLE_OK != status) {
		return -1;
	}
	return 0;
}

int
set_timeout(http_request_t* request, uint32_t secs) {
	CURLcode status = curl_easy_setopt(request->ctx, CURLOPT_TIMEOUT, secs);
	if (CURLE_OK != status) {
		return -1;
	}

	return 0;
}

const char*
get_http_headers(http_request_t* request, size_t* size) {
	*size = string_length(&request->receive_header);
	return string_str(&request->receive_header);
}

const char*
get_http_content(http_request_t* request, size_t* size) {
	*size = string_length(&request->receive_content);
	return string_str(&request->receive_content);
}

const char*
get_http_error(http_request_t* request) {
	return request->error;
}

int
get_http_code(http_request_t* request) {
	int code;
	curl_easy_getinfo(request->ctx, CURLINFO_RESPONSE_CODE, &code);
	return code;
}
