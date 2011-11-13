#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h> 

#include "net_core.h"

#define STATIC_IP_LEN	32	// compatible ipv6

void 	net_set_keepalive(int fd, int idle_time, int interval, int count)
{
	int on_keep = 1;
	int s = -1;
	s = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on_keep, sizeof(on_keep));
	assert(s==0);
	s = setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &idle_time , sizeof(idle_time));
	assert(s==0);
	s = setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
	assert(s==0);
	s = setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count));
	assert(s==0);
}

//set SO_LINGER option(respond CLOSE_WAIT hold all socket)   
inline
void 	net_set_linger(int fd)
{
    struct linger        	optval;
	optval.l_onoff  = 0;  // src 1  
    optval.l_linger = 0;  // src 60
    setsockopt(fd, SOL_SOCKET, SO_LINGER, &optval, sizeof(struct linger));
}

//set SO_REUSEADDR option(the server can reboot fast)   
inline
void	net_set_reuse_addr(int fd)
{
	int optval = 0x1;   
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
}

inline
void 	net_set_nonblocking(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

inline
void	net_set_recv_buffsize(int fd, int size)
{
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(int));
}

inline
void	net_set_send_buffsize(int fd, int size)
{
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(int));
}

// return -1 has error
inline
int		net_set_nodely(int fd){
	int flag = 1;
	return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
}

inline
void	net_set_recv_timeout(int fd, int timeout)
{
  	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
}

void	net_set_send_timeout(int fd, int timeout)
{
	struct timeval timeo = {2, 0};
	socklen_t len = sizeof(timeo);
	timeo.tv_sec = timeout;
  	int s = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);
	assert(s == 0);
}

inline
int		net_epoll_create(int queue_num)
{
    return epoll_create( queue_num );   
}

int		net_epoll_add(int epfd, int sockfd, int opt, void* ptr)
{
	struct epoll_event 	ev;

	if( opt == FREAD )
    	ev.events = EPOLLIN;
	else if ( opt == FWRITE )
    	ev.events = EPOLLOUT;
	else	// warning to use it
    	ev.events = EPOLLIN | EPOLLOUT;

	//ev.events |= EPOLLET;

    ev.data.ptr = ptr; 
	
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) < 0)
    {   
        printf("add epoll event error fd =%d errno =%d(%s)\n", sockfd, errno, strerror(errno));   
		return -1;
    }   

	return sockfd;
}

int		net_epoll_mod(int epfd, int fd, int opt, void* ptr)
{
	struct epoll_event 	ev;

	if( opt == FREAD )
		ev.events = EPOLLIN;
	else if ( opt == FWRITE )
		ev.events = EPOLLOUT;
	else 	// warning to use it
		ev.events = EPOLLIN | EPOLLOUT;

	//ev.events |= EPOLLET;
    ev.data.ptr = ptr; 

    return epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

inline
void	net_epoll_del(int epfd, int sockfd)
{
	struct epoll_event ev;
	ev.data.ptr = NULL;
	if( 0 != epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, &ev) )
		printf("net_core: epoll del failed efd=%d delsockfd=%d\n", epfd, sockfd);
}

int		net_create_listen(char* ip, int port, int max_link, int isblock)
{
    int            			listen_fd;   
    struct sockaddr_in    	addr;   
  
	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;   
    addr.sin_port = htons(port);

	if( ip )
		addr.sin_addr.s_addr = inet_addr(ip);
	else
		addr.sin_addr.s_addr = htons(INADDR_ANY);
   
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);   
    if (0 > listen_fd) return -1;   
  
	net_set_reuse_addr(listen_fd);
	net_set_linger(listen_fd);
    
	if( !isblock )
  		net_set_nonblocking(listen_fd);

    if (0 > bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)))   
    {   
        close(listen_fd);   
        return -1;   
    }   
  
    if (0 > listen(listen_fd, max_link))   
    {   
        close(listen_fd);   
        return -1;   
    }   
  
    return listen_fd;
}

inline
void	net_close(int fd)
{
	//shutdown(fd, SHUT_RDWR);   
	close(fd);
}

// send data util data all send complete
// when write return EAGAIN stop
int		net_send_safe(int fd, char* data, int len)
{
	int totalnum = 0;
	int sendnum = 0;

	while( totalnum < len )
	{
		sendnum = send(fd, data+totalnum, len-totalnum, MSG_NOSIGNAL);
		if( sendnum != -1 )
		{
			totalnum += sendnum;
			if( totalnum == len )
				return totalnum;
		}
		else
			if( errno == EAGAIN  )
				return totalnum;
			else if( errno == EINTR || errno == EWOULDBLOCK )
				continue;
			else{
				printf("errno = %d fd = %d\n reason=%s\n", errno, fd, strerror(errno));
				return -1;	// send error | that receiver has closed
			}
	}

	return totalnum;
}

// return real send num
inline
int 	net_send(int fd, const char* data, int len)
{
	do{
		int send_num = send(fd, data, len, MSG_NOSIGNAL);

		if( send >= 0 )
			return send_num;
		else
		{
			if( errno == EINTR )
				continue;
			else if( errno == EAGAIN )
				return SOCKET_IDLE;
			else if ( errno == EWOULDBLOCK )
				return SOCKET_BUSY;
			else
				return SOCKET_ERROR;
		}
	}while(1);
}

/*
inline
int		net_send(int fd, char* data, int len){
	int send_num = 0, total_num = 0;

	while( total_num < len ){
		send_num = send(fd, data+total_num, len-total_num, MSG_NOSIGNAL);
		if( send_num != -1 ){
			total_num += send_num;
			if( total_num == len )
				break;
		}
		else{
			if( errno == EINTR )
				continue;
			else if( errno == EAGAIN || errno == EWOULDBLOCK )
				break;
			else
				break;
		}
	}

	return total_num;
}
*/

// return <=0 the peer has quit, >0 can read
inline
int 	net_recv(int fd, char* data, int len)
{
	do{
		int recv_size = recv(fd, data, len, MSG_NOSIGNAL);

		if ( recv_size == -1 )
		{
			if( errno == EINTR )
				continue;
			else if( errno == EAGAIN )
				return SOCKET_IDLE;
			else if(errno == EWOULDBLOCK )
				return SOCKET_BUSY;
			return SOCKET_ERROR;
		}
		else if( recv_size == 0 )	// client quit
			return SOCKET_CLOSE;
		else
			return recv_size;
	}while(1);
}

uint	get_lowdata(uint data)
{
	return data & 0xFFFF;
}

uint	get_highdata(uint data)
{
	return ((data & 0xFFFF0000) >> 16) & 0xFFFF;
}

int		net_accept(int listen_fd)
{
	struct sockaddr		addr;
   	socklen_t addrlen = sizeof(struct sockaddr_in); 
	memset(&addr, 0, addrlen);

	int sock_fd = -1;
	do{
		sock_fd = accept(listen_fd, (struct sockaddr *)&addr, &addrlen);   

		if (sock_fd == -1)
			if( errno == EINTR )
				continue;
			else if( errno == EAGAIN || errno == EWOULDBLOCK )
				return SOCKET_IDLE;
			else
				return SOCKET_ERROR;
		else
			break;
	}while(0);

	net_set_recv_timeout(sock_fd, 2);

	return sock_fd;
}

int		net_conn(const char* ip, int port, int isblock)
{
	int 	sockfd; 
	struct 	sockaddr_in server_addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{ 
		printf("net_conn error:%s\a\n", strerror(errno)); 
		return	-1; 
	}

	if( !isblock )
  		net_set_nonblocking(sockfd);

	memset(&server_addr, 0, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_port = htons(port); 
	server_addr.sin_addr.s_addr = inet_addr(ip); 

	if( connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1 ) 
	{ 
		printf("net_conn:Connect Error:%s\a\n", strerror(errno)); 
		return	-1; 
	}
	printf("net_conn:connect sucess fd = %d\n", sockfd);
	
	return	sockfd;
}

// async connect, nonblocking
// return:
// 0: sucess, you can use outfd
// -1: error
// 1: connect has in process
int		net_conn_a(const char* ip, int port, int* outfd)
{
	int 	sockfd; 
	struct 	sockaddr_in server_addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{ 
		printf("net_conn_a error:%s\a\n", strerror(errno)); 
		return	-1; 
	}

	*outfd = sockfd;
  	net_set_nonblocking(sockfd);

	memset(&server_addr, 0, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_port = htons(port); 
	server_addr.sin_addr.s_addr = inet_addr(ip); 

	int r = connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr));
	if( r == -1 )
	{ 
		if( errno == EINPROGRESS )
			return 1;
		else{
			printf("lnet_conn:Connect Error:%s\a\n", strerror(errno)); 
			return	-1; 
		}
	}
	printf("lnet_conn:connect sucess fd = %d\n", sockfd);

	return	0;
}

void	net_create_event(event_data* ed, int event_num,
						pfunc_read pr, pfunc_write pw, 
						pfunc_error pe, pfunc_timeout pt){
	ed->events_list = malloc(sizeof(struct epoll_event) * event_num);
	ed->events_len = event_num;

	ed->pread = pr;
	ed->pwrite = pw;
	ed->perror = pe;
	ed->ptimeout = pt;
}

void	net_delete_event(event_data* ed){
	free(ed->events_list);
	ed->pread = NULL;
	ed->pwrite = NULL;
	ed->perror = NULL;
	ed->ptimeout = NULL;

	free(ed);
}

void	net_epoll_wait(int epfd, int timeout, event_data* ed){
	if( !ed ) return;
	int nfds = 0, i = 0;

	struct epoll_event* events = (struct epoll_event*)ed->events_list;
	nfds = epoll_wait(epfd, events, ed->events_len, timeout);  

	for (i=0; i<nfds; ++i){
		void* data = events[i].data.ptr;

		if( events[i].events & (EPOLLHUP | EPOLLRDHUP) ){
			if( ed->perror && ed->perror(data) ){}
			continue;
		}

		if( events[i].events & EPOLLIN )
			if( ed->pread && ed->pread(data)){}

		if( events[i].events & EPOLLOUT )
			if( ed->pwrite && ed->pwrite(data) ){}
	}

	// time out
	if( nfds == 0 )
		if( ed->ptimeout && ed->ptimeout() ){}
}

char* 	net_get_localip(int fd)
{
	sockaddr_u_t u_addr;
	socklen_t addr_len;

	getsockname(fd, &u_addr.sa, &addr_len);
	return inet_ntoa( u_addr.in.sin_addr );
}

char*	net_get_peerip(int fd)
{
	sockaddr_u_t u_addr;
	socklen_t addr_len;

	getpeername(fd, &u_addr.sa, &addr_len);
	return inet_ntoa( u_addr.in.sin_addr );
}

int		net_get_host(const char* host_name, host_info_t* hinfo)
{
	if( !host_name ) return 1;
	hinfo->alias_count = 0;
	hinfo->ip_count = 0;
	hinfo->official_name = NULL;
	hinfo->alias_names = NULL;
	hinfo->ips = NULL;

	const char* ptr;
	char** pptr; 
	struct hostent *hptr; 
	ptr = host_name;

	if((hptr = gethostbyname(ptr)) == NULL) {
		  return 2;	// failed to get host info
	}
						
	int host_name_len = strlen(hptr->h_name) + 1;
	char* h_name = (char*)malloc(host_name_len);
	hinfo->official_name = strncpy(h_name, hptr->h_name, host_name_len);

	for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
		hinfo->alias_count++;

	int i = 0;
	hinfo->alias_names = (char**)malloc(sizeof(char*) * hinfo->alias_count);
	for(pptr = hptr->h_aliases; *pptr != NULL && i < hinfo->alias_count; pptr++, i++){
		int len = strlen(*pptr) + 1;
		char* alias_name = (char*)malloc(len);
		hinfo->alias_names[i] = strncpy(alias_name, *pptr, len);
	}
						  
	switch(hptr->h_addrtype) {
		case AF_INET: 
		case AF_INET6: 
			pptr = hptr->h_addr_list; 
			for(; *pptr!=NULL; pptr++) 
				hinfo->ip_count++;

			pptr = hptr->h_addr_list; 
			hinfo->ips = (char**)malloc(sizeof(char*) * hinfo->ip_count);
			for(i=0; *pptr!=NULL && i < hinfo->ip_count; pptr++, i++){
				char* ip = (char*)malloc(STATIC_IP_LEN);
				inet_ntop(hptr->h_addrtype, *pptr, ip, STATIC_IP_LEN);
				hinfo->ips[i] = ip;
			}

			break;
		default:
			printf("unknown address type\n");
			return 3;
	}

	return 0;
}

void	net_free_host(host_info_t* hinfo)
{
	int i;
	free(hinfo->official_name);

	for(i = 0; i < hinfo->alias_count; ++i )
		free(hinfo->alias_names[i]);
	free(hinfo->alias_names);

	for(i = 0; i < hinfo->ip_count; ++i )
		free(hinfo->ips[i]);
	free(hinfo->ips);
}
