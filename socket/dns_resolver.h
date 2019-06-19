#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

#include "socket_tcp.h"
#include "socket_util.h"

typedef void(*dns_resolve_result)(int status, struct hostent *host, const char* reason, void* ud);

struct dns_resolver;


struct dns_resolver* dns_resolver_new(struct ev_loop_ctx* ev_loop);
void dns_resolver_delete(struct dns_resolver* resolver);

void dns_query(struct dns_resolver* resolver, const char* name, dns_resolve_result cb, void* ud);
const char* dns_last_error(int status);

#endif