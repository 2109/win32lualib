#ifndef SOCKET_UTIL_H
#define SOCKET_UTIL_H
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <WinSock2.h>

#define HOST_SIZE 128


union sockaddr_all {
	struct sockaddr s;
	struct sockaddr_in v4;
};

#define SOCKET_OPT_NOBLOCK				(1u<<0)
#define SOCKET_OPT_CLOSE_ON_EXEC		(1u<<1)
#define SOCKET_OPT_REUSEABLE_ADDR		(1u<<2)
#define SOCKET_OPT_REUSEABLE_PORT		(1u<<3)

int socket_nonblock(int fd, bool nonblocking);
int socket_no_delay(int fd);
int socket_keep_alive(int fd);
int socket_reuse_addr(int fd);
int socket_recv_buffer(int fd, int size);
int socket_send_buffer(int fd, int size);

int socket_connect(struct sockaddr* addr, int addrlen, int block, int* connected);
int socket_listen(struct sockaddr* addr, int addrlen, int backlog, int flag);
int socket_bind(struct sockaddr *addr, int addrlen, int flag, int protocol);
int socket_accept(int fd, char* info, size_t length);
int socket_read(int fd, char* data, size_t size);
int socket_write(int fd, char* data, size_t size);
int socket_udp_write(int fd, char* data, size_t size, struct sockaddr* addr, size_t addrlen);
int socket_pipe_write(int fd, void* data, size_t size);
int get_peername(int fd, char* out, size_t out_len, int* port);
int get_sockname(int fd, char* out, size_t out_len, int* port);

#endif