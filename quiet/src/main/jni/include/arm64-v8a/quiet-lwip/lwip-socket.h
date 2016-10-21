#ifndef QUIET_LWIP_SOCKET_H
#define QUIET_LWIP_SOCKET_H
#include <stdint.h>
#include <netinet/in.h>

struct lwip_in_addr {
    uint32_t s_addr;
};

struct lwip_sockaddr_in {
  uint8_t sin_len;
  uint8_t sin_family;
  uint16_t sin_port;
  struct lwip_in_addr sin_addr;
  char sin_zero[8];
};

struct lwip_sockaddr {
  uint8_t sa_len;
  uint8_t sa_family;
  char sa_data[14];
};

typedef uint32_t lwip_socklen_t;

int lwip_accept(int s, struct lwip_sockaddr *addr, lwip_socklen_t *addrlen);
int lwip_bind(int s, const struct lwip_sockaddr *name, lwip_socklen_t namelen);
int lwip_shutdown(int s, int how);
int lwip_getpeername (int s, struct lwip_sockaddr *name, lwip_socklen_t *namelen);
int lwip_getsockname (int s, struct lwip_sockaddr *name, lwip_socklen_t *namelen);
int lwip_getsockopt (int s, int level, int optname, void *optval, lwip_socklen_t *optlen);
int lwip_setsockopt (int s, int level, int optname, const void *optval, lwip_socklen_t optlen);
int lwip_close(int s);
int lwip_connect(int s, const struct lwip_sockaddr *name, lwip_socklen_t namelen);
int lwip_listen(int s, int backlog);
int lwip_recv(int s, void *mem, size_t len, int flags);
int lwip_read(int s, void *mem, size_t len);
int lwip_recvfrom(int s, void *mem, size_t len, int flags,
      struct lwip_sockaddr *from, lwip_socklen_t *fromlen);
int lwip_send(int s, const void *dataptr, size_t size, int flags);
int lwip_sendto(int s, const void *dataptr, size_t size, int flags,
    const struct lwip_sockaddr *to, lwip_socklen_t tolen);
int lwip_socket(int domain, int type, int protocol);
int lwip_write(int s, const void *dataptr, size_t size);
int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                struct timeval *timeout);
int lwip_ioctl(int s, long cmd, void *argp);
int lwip_fcntl(int s, int cmd, int val);
#endif
