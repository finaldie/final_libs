#ifndef FNET_H
#define FNET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>

#define SOCKET_IDLE     -2
#define SOCKET_ERROR    -1
#define SOCKET_CLOSE    0

// This feature only enabled when linux kernel >= 3.9
#ifndef SO_REUSEPORT
#define SO_REUSEPORT 15
#endif

// host struct
typedef struct {
    int    alias_count;
    int    ip_count;
    char*  official_name;
    char** alias_names;
    char** ips;
} fhost_info_t;

// Union of socketaddr
typedef union fnet_sockaddr_t {
    struct sockaddr_in  in;
    struct sockaddr_in6 in6;
    struct sockaddr     sa;
    struct sockaddr_storage ss;
} fnet_sockaddr_t;

unsigned int fnet_get_lowdata(unsigned int data);
unsigned int fnet_get_highdata(unsigned int data);

int     fnet_set_keepalive(int fd, int idle_time, int interval, int count);
int     fnet_set_nonblocking(int fd);
int     fnet_set_nodelay(int fd);
int     fnet_set_recv_buffsize(int fd, int size);
int     fnet_set_send_buffsize(int fd, int size);
int     fnet_set_recv_timeout(int fd, int timeout);
int     fnet_set_send_timeout(int fd, int timeout);
int     fnet_set_linger(int fd);
int     fnet_set_reuse_addr(int fd);
int     fnet_set_reuse_port(int fd);

/**
 * Currently support IPv4/IPv6 socket
 */
int     fnet_socket(const char* ip, in_port_t port, int type, fnet_sockaddr_t*,
                    socklen_t* len);
int     fnet_listen(const char* ip, in_port_t port, int backlog, int isblock);
int     fnet_accept(int listen_fd);
ssize_t fnet_send(int fd, const void* data, size_t len);
ssize_t fnet_send_safe(int fd, const void* data, size_t len);
ssize_t fnet_recv(int fd, void* data, size_t len);
int     fnet_conn(const char* ip, in_port_t port, int isblock);
int     fnet_conn_async(const char* ip, in_port_t port, int* outfd);

const char* fnet_sockname(int fd, char* buf, socklen_t size, int* port, int* family);
const char* fnet_peername(int fd, char* buf, socklen_t size, int* port, int* family);

// dns , after use must call net_free_host
int     fnet_get_host(const char* host_name, fhost_info_t* hinfo);
void    fnet_free_host(fhost_info_t* hinfo);

#ifdef __cplusplus
}
#endif

#endif

