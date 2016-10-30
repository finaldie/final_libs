#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "flibs/fnet.h"

#if (EAGAIN != EWOULDBLOCK)
# define NEED_RETRY \
    EAGAIN: \
    case EWOULDBLOCK
#else
# define NEED_RETRY EAGAIN
#endif

#define STATIC_IP_LEN    32    // compatible ipv6

// use it avoid gcc throw error 'break strict-aliasing rules'
typedef union {
    struct sockaddr_storage storage;
    struct sockaddr_in      in;
    struct sockaddr_in6     in6;
    struct sockaddr         sa;
} sockaddr_u_t;

int     fnet_set_keepalive(int fd, int idle_time, int interval, int count)
{
    int on_keep = 1;
    int s = -1;
    s = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on_keep, sizeof(on_keep));
    if (s) return s;

    s = setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle_time, sizeof(idle_time));
    if (s) return s;

    s = setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    if (s) return s;

    return setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));
}

//set SO_LINGER option(respond CLOSE_WAIT hold all socket)
inline
int     fnet_set_linger(int fd)
{
    struct linger            optval;
    optval.l_onoff  = 0;  // src 1
    optval.l_linger = 0;  // src 60
    return setsockopt(fd, SOL_SOCKET, SO_LINGER, &optval, sizeof(struct linger));
}

//set SO_REUSEADDR option(the server can reboot fast)
inline
int    fnet_set_reuse_addr(int fd)
{
    int optval = 0x1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
}

//set SO_REUSEPORT option(different fds can bind on the same address and same port)
//both suit for TCP and UDP
//to enable this, need also enable kernel support it
//before start program, run the command as below:
//sysctl net.core.allow_reuseport=1
inline
int    fnet_set_reuse_port(int fd)
{
    (void)fd;
#ifdef SO_REUSEPORT
    int optval = 0x1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
#else
    return 0;
#endif
}

int     fnet_set_nonblocking(int fd)
{
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

inline
int     fnet_set_recv_buffsize(int fd, int size)
{
    return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int));
}

inline
int     fnet_set_send_buffsize(int fd, int size)
{
    return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(int));
}

// return -1 has error
inline
int     fnet_set_nodely(int fd){
    int flag = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
}

int     fnet_set_recv_timeout(int fd, int timeout)
{
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
}

int     fnet_set_send_timeout(int fd, int timeout)
{
    struct timeval timeo = {2, 0L};
    socklen_t len = sizeof(timeo);
    timeo.tv_sec = timeout;
    return setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
}

int     fnet_listen(const char* ip, in_port_t port, int max_link, int isblock)
{
    int listen_fd;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if ( ip )
        addr.sin_addr.s_addr = inet_addr(ip);
    else
        addr.sin_addr.s_addr = htons(INADDR_ANY);

    listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (0 > listen_fd) return -1;

    if (fnet_set_reuse_addr(listen_fd)) {
        goto cleanup;
    }

    if (fnet_set_linger(listen_fd)) {
        goto cleanup;
    }

    if ( !isblock && fnet_set_nonblocking(listen_fd) ) {
        goto cleanup;
    }

    if (0 > bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr))) {
        goto cleanup;
    }

    if (0 > listen(listen_fd, max_link)) {
        goto cleanup;
    }

    return listen_fd;

cleanup:
    close(listen_fd);
    return -1;
}

// send data util all data be sent completely
// NOTES: stop when write return EAGAIN or EWOULDBLOCK
ssize_t fnet_send_safe(int fd, const void* data, size_t len)
{
    ssize_t totalnum = 0;
    ssize_t sendnum = 0;

    while ( (size_t)totalnum < len ) {
        sendnum = send(fd, (char*)data + totalnum, len - (size_t)totalnum, MSG_NOSIGNAL);
        if (sendnum != -1) {
            totalnum += sendnum;

            if ((size_t)totalnum == len) {
                return totalnum;
            }
        } else {
            switch (errno) {
            case NEED_RETRY:
                return totalnum;
            case EINTR:
                continue;
            default:
                return -1;
            }
        }
    }

    return totalnum;
}

// return real send num
ssize_t fnet_send(int fd, const void* data, size_t len)
{
    do {
        ssize_t send_num = send(fd, data, len, MSG_NOSIGNAL);

        if ( send_num >= 0 )
            return send_num;
        else {
            switch (errno) {
            case EINTR:
                continue;
            case NEED_RETRY:
                return SOCKET_IDLE;
            default:
                return SOCKET_ERROR;
            }
        }
    } while (1);
}

// return <=0 the peer has quit, >0 can read
ssize_t fnet_recv(int fd, void* data, size_t len)
{
    do {
        ssize_t recv_size = recv(fd, data, len, MSG_NOSIGNAL);

        if (recv_size == -1) {
            switch (errno) {
            case EINTR:
                continue;
            case NEED_RETRY:
                return SOCKET_IDLE;
            default:
                return SOCKET_ERROR;
            }
        } else if ( recv_size == 0 ) { // peer quit
            return SOCKET_CLOSE;
        } else {
            return recv_size;
        }
    } while (1);
}

unsigned int fnet_get_lowdata(unsigned int data)
{
    return data & 0xFFFF;
}

unsigned int fnet_get_highdata(unsigned int data)
{
    return ((data & 0xFFFF0000) >> 16) & 0xFFFF;
}

int     fnet_accept(int listen_fd)
{
    struct sockaddr addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    memset(&addr, 0, addrlen);

    int sock_fd = -1;
    do {
        sock_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);

        if (sock_fd == -1) {
            switch (errno) {
            case EINTR:
                continue;
            case NEED_RETRY:
                return SOCKET_IDLE;
            default:
                return SOCKET_ERROR;
            }
        } else break;
    } while (1);

    return sock_fd;
}

// Sync method for connect
// note: Whether or not set block type after connect sucess
int     fnet_conn(const char* ip, in_port_t port, int isblock)
{
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) == -1) {
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if ( connect(sockfd, (struct sockaddr *)(&server_addr),
        sizeof(struct sockaddr)) == -1 ) {
        return -1;
    }

    if ( !isblock )
        fnet_set_nonblocking(sockfd);

    return sockfd;
}

// async connect, nonblocking
// return:
// 0: sucess, you can use outfd
// -1: error
// 1: connect has in process
int     fnet_conn_async(const char* ip, in_port_t port, int* outfd)
{
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) == -1) {
        return -1;
    }

    *outfd = sockfd;
    fnet_set_nonblocking(sockfd);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    int r = connect(sockfd, (struct sockaddr *)(&server_addr),
                    sizeof(struct sockaddr));
    if ( r == -1 ) {
        if ( errno == EINPROGRESS ) {
            return 1;
        } else {
            return -1;
        }
    }

    return 0;
}

char*   fnet_get_localip(int fd)
{
    sockaddr_u_t u_addr;
    socklen_t addr_len;

    if ( !getsockname(fd, &u_addr.sa, &addr_len) ) {
        return inet_ntoa( u_addr.in.sin_addr );
    } else {
        return NULL;
    }
}

char*   fnet_get_peerip(int fd)
{
    sockaddr_u_t u_addr;
    socklen_t addr_len;

    if ( !getpeername(fd, &u_addr.sa, &addr_len) ) {
        return inet_ntoa( u_addr.in.sin_addr );
    } else {
        return NULL;
    }
}

int     fnet_get_host(const char* host_name, fhost_info_t* hinfo)
{
    if (!host_name) return 1;
    hinfo->alias_count = 0;
    hinfo->ip_count = 0;
    hinfo->official_name = NULL;
    hinfo->alias_names = NULL;
    hinfo->ips = NULL;

    const char* ptr;
    char** pptr;
    struct hostent *hptr;
    ptr = host_name;

    if ((hptr = gethostbyname(ptr)) == NULL) {
        return 2;    // failed to get host info
    }

    size_t host_name_len = strlen(hptr->h_name) + 1;
    char* h_name = calloc(1, host_name_len);
    hinfo->official_name = strncpy(h_name, hptr->h_name, host_name_len);

    for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)
        hinfo->alias_count++;

    hinfo->alias_names = (char**)calloc(1, sizeof(char*) * (size_t)hinfo->alias_count);
    int i = 0;
    for (pptr = hptr->h_aliases;
         *pptr != NULL && i < hinfo->alias_count;
         pptr++, i++) {
        size_t len = strlen(*pptr) + 1;
        char* alias_name = calloc(1, len);
        hinfo->alias_names[i] = strncpy(alias_name, *pptr, len);
    }

    switch (hptr->h_addrtype) {
        case AF_INET:
        case AF_INET6:
            pptr = hptr->h_addr_list;
            for ( ; *pptr!=NULL; pptr++)
                hinfo->ip_count++;

            pptr = hptr->h_addr_list;
            hinfo->ips = (char**)calloc(1, sizeof(char*) * (size_t)hinfo->ip_count);
            for (i=0; *pptr!=NULL && i < hinfo->ip_count; pptr++, i++) {
                char* ip = calloc(1, STATIC_IP_LEN);
                inet_ntop(hptr->h_addrtype, *pptr, ip, STATIC_IP_LEN);
                hinfo->ips[i] = ip;
            }

            break;
        default:
            return 3;
    }

    return 0;
}

void    fnet_free_host(fhost_info_t* hinfo)
{
    int i;
    free(hinfo->official_name);

    for ( i = 0; i < hinfo->alias_count; ++i )
        free(hinfo->alias_names[i]);
    free(hinfo->alias_names);

    for ( i = 0; i < hinfo->ip_count; ++i )
        free(hinfo->ips[i]);
    free(hinfo->ips);
}
