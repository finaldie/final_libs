#ifndef _NET_UTIL_H_
#define	_NET_UTIL_H_

#include <netinet/in.h>
#include "core.h"

#define SOCKET_IDLE		-3
#define SOCKET_ERROR 	-2
#define SOCKET_BUSY		-1
#define	SOCKET_CLOSE	0

#define FREAD			0x1
#define FWRITE			0x2

typedef int	(*pfunc_read)(void* data);
typedef	int (*pfunc_write)(void* data);
typedef	int (*pfunc_error)(void* data);
typedef	int (*pfunc_timeout)(void);

// net call back
typedef struct{
	void*			events_list;
	int				events_len;

	pfunc_read 		pread;
	pfunc_write 	pwrite;
	pfunc_error 	perror;
	pfunc_timeout 	ptimeout;
}event_data;

// host struct
typedef struct{
	int		alias_count;
	int 	ip_count;
	char* 	official_name;
	char**	alias_names;
	char**	ips;		
}host_info_t;

// use it avoid gcc throw error 'break strict-aliasing rules'
typedef union{
	struct sockaddr_storage storage;
	struct sockaddr_in 		in;
	struct sockaddr_in6 	in6;
	struct sockaddr 		sa;
}sockaddr_u_t;

uint	get_lowdata(uint data);
uint	get_highdata(uint data);

void 	net_set_keepalive(int fd, int idle_time, int interval, int count);
void	net_set_nonblocking(int fd);
int		net_set_nodely(int fd);
void	net_set_recv_buffsize(int fd, int size);
void	net_set_send_buffsize(int fd, int size);
void	net_set_recv_timeout(int fd, int timeout);
void	net_set_send_timeout(int fd, int timeout);
void 	net_set_linger(int fd);
void	net_set_reuse_addr(int fd);

int		net_epoll_create(int queue_num);
int		net_epoll_add(int epfd, int fd, int opt, void* ptr);
int		net_epoll_mod(int epfd, int fd, int opt, void* ptr);
void	net_epoll_del(int epfd, int fd);

void	net_create_event(event_data*, int event_num, 
						pfunc_read, pfunc_write, 
						pfunc_error, pfunc_timeout);
void	net_delete_event(event_data*);
void	net_epoll_wait(int epfd, int timeout, event_data* ed);

int		net_create_listen(char* ip, int port, int max_link, int isblock);
int		net_accept(int listen_fd);
void	net_close(int fd);
int 	net_send(int fd, const char* data, int len);
int 	net_recv(int fd, char* data, int len);
int		net_conn(const char* ip, int port, int isblock);
int		net_conn_a(const char* ip, int port, int* outfd);
int		net_send_safe(int fd, char* data, int len);

char*	net_get_localip(int fd);
char*	net_get_peerip(int fd);

// dns , after use must call net_free_host
int		net_get_host(const char* host_name, host_info_t* hinfo);
void	net_free_host(host_info_t* hinfo);
#endif
