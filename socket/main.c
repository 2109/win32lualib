#include <stdio.h>
#include <stdlib.h>
#include "socket_tcp.h"
#include "socket_httpc.h"
#include "dns_resolver.h"

static void
accept_complete(struct ev_listener *listener, int fd, const char* addr, void *ud) {

}

int count = 0;
void dns_query_callback(int status, struct hostent *host, const char* reason, void* ud) {
	printf("%d,%s\n", ++count, reason);
	//char ip[INET6_ADDRSTRLEN];
	//int i;
	//for ( i = 0; host->h_addr_list[i]; ++i ) {
	//	inet_ntop(host->h_addrtype, host->h_addr_list[i], ip, sizeof(ip));
	//}
	/*struct dns_resolver* resolver = (struct dns_resolver*)ud;
	dns_query(resolver, "www.baidu.com", dns_query_callback, ud);*/
}

static void request_done(struct http_request* request, int code, void* ud) {
	size_t size;
	const char* content = get_http_content(request, &size);
	printf("%d, %s, %s\r\n", code, content, get_http_error(request));
}

int main() {
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
	struct ev_loop_ctx* loop = loop_ctx_create();

	struct ev_listener* listener = ev_listener_bind_ipv4(loop, "127.0.0.1", 1989, accept_complete, NULL);

	struct dns_resolver* resolver = dns_resolver_new(loop);
	struct http_multi* multi = http_multi_new(loop);

	for ( int i = 1; i < 1024; i++ ) {
		dns_query(resolver, "www.baidu.com", dns_query_callback, resolver);
		dns_query(resolver, "www.163.com", dns_query_callback, resolver);
	}

	//dns_query(resolver, "www.baidu.com", dns_query_callback, resolver);

	//int i;
	//for ( i = 0; i < 10000; i++ ) {
	//	struct http_request* request = http_request_new();
	//	set_url(request, "148.70.76.220:8888/a/b/c?a=1&b=2");
	//	http_request_perform(multi, request, request_done, NULL);
	//}
	//
	loop_ctx_dispatch(loop);
	return 0;
}