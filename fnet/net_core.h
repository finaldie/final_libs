#ifndef _NET_UTIL_H_
#define    _NET_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>

#define SOCKET_IDLE     -2
#define SOCKET_ERROR    -1
#define SOCKET_CLOSE    0

// host struct
typedef struct{
    int    alias_count;
    int    ip_count;
    char*  official_name;
    char** alias_names;
    char** ips;
}host_info_t;

// use it avoid gcc throw error 'break strict-aliasing rules'
typedef union{
    struct sockaddr_storage storage;
    struct sockaddr_in      in;
    struct sockaddr_in6     in6;
    struct sockaddr         sa;
}sockaddr_u_t;

unsigned int get_lowdata(unsigned int data);
unsigned int get_highdata(unsigned int data);

void    net_set_keepalive(int fd, int idle_time, int interval, int count);
void    net_set_nonblocking(int fd);
int     net_set_nodely(int fd);
void    net_set_recv_buffsize(int fd, int size);
void    net_set_send_buffsize(int fd, int size);
void    net_set_recv_timeout(int fd, int timeout);
void    net_set_send_timeout(int fd, int timeout);
void    net_set_linger(int fd);
void    net_set_reuse_addr(int fd);
void    net_set_reuse_port(int fd);

int     net_create_listen(char* ip, int port, int max_link, int isblock);
int     net_accept(int listen_fd);
void    net_close(int fd);
int     net_send(int fd, const void* data, int len);
int     net_recv(int fd, char* data, int len);
int     net_conn(const char* ip, int port, int isblock);
int     net_conn_a(const char* ip, int port, int* outfd);
int     net_send_safe(int fd, const void* data, int len);

char*   net_get_localip(int fd);
char*   net_get_peerip(int fd);

// dns , after use must call net_free_host
int     net_get_host(const char* host_name, host_info_t* hinfo);
void    net_free_host(host_info_t* hinfo);

#ifdef __cplusplus
}
#endif

#endif
