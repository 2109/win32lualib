#include "socket_util.h"
#include <ws2ipdef.h>
#include <WS2tcpip.h>
int socket_nonblock(int fd, bool nonblocking) {
#if defined( WIN32 )
	u_long val = nonblocking ? 1 : 0;
	return ioctlsocket(fd, FIONBIO, &val);
#else
	int val = nonblocking ? O_NONBLOCK : 0;
	return fcntl(fd, F_SETFL, val);
#endif
}

int socket_no_delay(int fd) {
	int on = 1;
	return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&on, sizeof(on));
}

int socket_keep_alive(int fd) {
	int keepalive = 1;
	return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepalive, sizeof(keepalive));
}

int socket_reuse_addr(int fd) {
	int one = 1;
	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&one, sizeof(one));
}

int socket_recv_buffer(int fd, int size) {
	return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

int socket_send_buffer(int fd, int size) {
	return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

int
socket_bind(struct sockaddr *addr, int addrlen, int flag, int protocol) {
	int fd;

	if ( protocol == IPPROTO_TCP ) {
		fd = socket(addr->sa_family, SOCK_STREAM, 0);
	}
	else {
		assert(protocol == IPPROTO_UDP);
		fd = socket(addr->sa_family, SOCK_DGRAM, 0);
	}

	if ( fd < 0 )
		return -1;

	if ( flag & SOCKET_OPT_NOBLOCK ) {
		if ( socket_nonblock(fd, true) == -1 ) {
			closesocket(fd);
			return -1;
		}
	}

	if ( flag & SOCKET_OPT_REUSEABLE_ADDR ) {
		if ( socket_reuse_addr(fd) == -1 ) {
			closesocket(fd);
			return -1;
		}
	}

	int status = bind(fd, addr, addrlen);
	if ( status != 0 )
		return -1;
	return fd;
}

int
socket_listen(struct sockaddr* addr, int addrlen, int backlog, int flag) {
	int listen_fd = socket_bind(addr, addrlen, flag, IPPROTO_TCP);
	if ( listen_fd < 0 ) {
		return -1;
	}
	if ( listen(listen_fd, backlog) == -1 ) {
		closesocket(listen_fd);
		return -1;
	}

	return listen_fd;
}

int
socket_connect(struct sockaddr* addr, int addrlen, int block, int* connected) {
	int fd = socket(addr->sa_family, SOCK_STREAM, 0);
	if ( fd < 0 )
		return -1;

	socket_keep_alive(fd);

	int status;
	if ( !block ) {
		socket_nonblock(fd, true);
		status = connect(fd, addr, addrlen);
		if ( status != 0 && errno != EINPROGRESS ) {
			closesocket(fd);
			return -1;
		}
		if ( status == 0 ) {
			*connected = 1;
			return fd;
		}
		*connected = 0;
	}
	else {
		status = connect(fd, addr, addrlen);
		if ( status != 0 ) {
			closesocket(fd);
			return -1;
		}
		socket_nonblock(fd, true);
		*connected = 1;
	}

	return fd;
}

int
socket_read(int fd, char* data, size_t len) {
	size_t offset = 0;
	size_t left = len;
	while ( left > 0 ) {
		int n = (int)read(fd, data + offset, left);
		if ( n < 0 ) {
			if ( errno ) {
				if ( errno == EINTR ) {
					continue;
				}
				else if ( errno == EAGAIN ) {
					break;
				}
				else {
					return -1;
				}
			}
			else {
				assert(0);
			}
		}
		else if ( n == 0 ) {
			return -1;
		}
		else {
			offset += n;
			left -= n;
		}
	}
	return offset;
}

int
socket_write(int fd, char* data, size_t size) {
	int total = 0;
	for ( ;; ) {
		int sz = (int)write(fd, data, size);
		if ( sz < 0 )  {
			switch ( errno )
			{
				case EINTR:
					continue;
				case EAGAIN:
					return total;
				default:
					fprintf(stderr, "send fd :%d error:%s\n", fd, strerror(errno));
					return -1;
			}
		}
		else if ( sz == 0 ) {
			return -1;
		}
		else {
			size -= sz;
			data += sz;
			total += sz;
			if ( 0 == size )
				break;
		}
	}

	return total;
}

int
socket_udp_write(int fd, char* data, size_t size, struct sockaddr* addr, size_t addrlen) {
	int total = 0;
	for ( ;; ) {
		int sz = sendto(fd, data, size, 0, (struct sockaddr *)addr, addrlen);
		if ( sz < 0 ) {
			switch ( errno )
			{
				case EINTR:
					continue;
				case EAGAIN:
					return total;
				default:
					fprintf(stderr, "sendto fd :%d error:%s\n", fd, strerror(errno));
					return -1;
			}
		}
		else if ( sz == 0 ) {
			return -1;
		}
		else {
			size -= sz;
			data += sz;
			total += sz;
			if ( 0 == size )
				break;
		}
	}
	return total;
}

int
socket_accept(int listen_fd, char* info, size_t length) {
	union sockaddr_all u;
	socklen_t len = sizeof(u);
	int client_fd = accept(listen_fd, &u.s, &len);
	if ( client_fd < 0 ) {
		_snprintf(info, length, "%s", strerror(errno));
		return -1;
	}

	socket_keep_alive(client_fd);
	socket_nonblock(client_fd, true);

	if ( u.s.sa_family == AF_INET ) {
		void * sin_addr = (void*)&u.v4.sin_addr;
		int sin_port = ntohs(u.v4.sin_port);
		char tmp[INET_ADDRSTRLEN];
		if ( inet_ntop(u.s.sa_family, sin_addr, tmp, sizeof(tmp)) ) {
			_snprintf(info, length, "%s:%d", tmp, sin_port);
		}
	}
	else {
		_snprintf(info, length, "ipc:unknown");
	}

	return client_fd;
}

int
socket_pipe_write(int fd, void* data, size_t size) {
	for ( ;; ) {
		int n = write(fd, data, size);
		if ( n < 0 ) {
			if ( errno == EINTR ) {
				continue;
			}
			else if ( errno == EAGAIN ) {
				return -1;
			}
			else {
				fprintf(stderr, "pipe_session_write_fd error %s.\n", strerror(errno));
				assert(0);
			}
		}
		assert(n == size);
		break;
	}
	return 0;
}

int
get_peername(int fd, char* out, size_t out_len, int* port) {
	union sockaddr_all u;
	socklen_t slen = sizeof(u);
	if ( getpeername(fd, &u.s, &slen) != 0 )
		return -1;

	void * sin_addr = (void*)&u.v4.sin_addr;
	int sin_port = ntohs(u.v4.sin_port);
	if ( port )
		*port = sin_port;

	if ( inet_ntop(u.s.sa_family, sin_addr, out, out_len) )
		return 0;

	return -1;
}

int
get_sockname(int fd, char* out, size_t out_len, int* port) {
	union sockaddr_all u;
	socklen_t slen = sizeof(u);
	if ( getsockname(fd, &u.s, &slen) != 0 )
		return -1;
	if ( u.s.sa_family == AF_INET || u.s.sa_family == AF_INET6 ) {
		void * sin_addr = (void*)&u.v4.sin_addr;
		int sin_port = ntohs(u.v4.sin_port);
		if ( port )
			*port = sin_port;

		if ( inet_ntop(u.s.sa_family, sin_addr, out, out_len) )
			return 0;
	}
	else if ( u.s.sa_family == AF_UNIX ) {
		return 0;
	}

	return -1;
}