//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>

#include "net_core.h"
#include "libnet.h"
#include "list.h"
#include "mbuf.h"
#include "lmutex.h"
#include "lhash.h"
#include "ltimer.h"

#define	 EPOLL_QUEUE_NUM		1000
#define	 MAXPOLLNUM				8192
#define	 LISTEN_QUEUE_NUM		64
#define	 EPOLL_TIME_OUT			500
#define  LNET_CONN_TIME_OUT		1000
#define  LNET_CONN_CHECK_ALTER	1
#define	 NET_BUFF_RECV_SIZE		2048
//#define	 LNET_READ_QUEUE_SIZE	32768
#define	 NET_BUFF_SEND_SIZE		32768 * 2
#define	 DEFAULT_CONN_HASH		128

#ifdef  _USE_SPIN_LOCK_
#define LOCK_INIT(lock) spin_init(lock)
#define DOLOCK(lock)	spin_lock(lock)
#define UNLOCK(lock)	spin_unlock(lock)
#define LOCK_DEL(lock)	spin_del(lock)
#else
#define LOCK_INIT(lock) mutex_init(lock)
#define DOLOCK(lock)	mutex_lock(lock)
#define UNLOCK(lock)	mutex_unlock(lock)
#define LOCK_DEL(lock)	mutex_del(lock)
#endif

#define TRUE    1
#define FALSE   0

typedef struct _net_interface
{
	int			r_epfd;		//read epfd
	int			g_vfd;		//vfd base count

	pl_mgr		plisten_list;

	cond_var 	r_cond;			// wake up the recv thread
	f_hash*		all_conn;
	f_hash*		ready2destroy;
	f_hash*		ev_pool;		// manage event obj
	
	size_t		recv_buf_size;	// default receive buff size
	char*		recv_buf;
	char*		write_buf;
	on_shutdown_conn pshut_down_func;
	etimer* 	ready2destroy_timer;

#ifdef _USE_SPIN_LOCK_
	spin_var 	r_lock;
	spin_var 	w_lock;
	spin_var	g_lock;
#else
	mutex_var 	r_lock;
	mutex_var 	w_lock;
	mutex_var	g_lock;
#endif

	spin_var	vfd_lock;
	spin_var	ref_lock;
}nif;

static nif* pnif = NULL;
static volatile int conn_thread_on = 0;

typedef struct{
	int			type;
	//int			vfd;
}check_head;

typedef struct 
{
	int			type;

	int			listen_fd;
	int			port;
	int			is_block;

	on_accept	accept;
}listen_info;

//notice:
//every type struct must contained check_head members
struct _net_buff
{
	int			type;

	int			fd;
	int			vfd;
	int			ip;
	int			port;
	int			ref_count;
	void*		ptr;
	int			event;
	int			ready2destroy;	// 0: not ready, 1: ready when call on_error set it
	mbuf*		w_buff;
	mbuf*		r_buff;
	size_t		w_buff_size;
	size_t		r_buff_size;

	int			r_active;
	int			w_active;

	on_recv 	recv;
	on_send		send;
	on_error	error;
};

struct _event_timer{
	int 		type;
	//int		vfd;

	int			fd;
	on_timer	pfunc;
	void*		arg;
};

typedef struct conn_event {
	int			type;

	int			fd;
	int			timeout;
	int			is_block;
	int			conn_time;	//start connect time

	pconn		pc;
	conn_arg_t 	arg;
}conn_ev;


// event type define
#define LNET_EVENT_TYPE_BUFF		0
#define LNET_EVENT_TYPE_TIMER		1
#define LNET_EVENT_TYPE_CONN		2
#define LNET_EVENT_TYPE_LISTEN		3

// event define
#define LNET_BUFF_EVENT_READ	0x1
#define LNET_BUFF_EVENT_NOTICE	0x2
#define LNET_BUFF_EVENT_CLOSE	0x4
#define LNET_BUFF_EVENT_POP		0x8

//TODO...
int		lnet_get_freelen(net_buff* nbuff, int type);
void	lnet_try_close(net_buff* nbuff);
void	lnet_proc_send(net_buff* nbuff, char* buff);
static
void	lnet_proc_user_close(net_buff* nbuff, int add);

static inline
void	lnet_set_event_type(void* data, int type){
	check_head* head = (check_head*)data;

	head->type = type;
	//head->vfd = vfd;
}

static inline
int		lnet_get_event_type(void* data){
	return ((check_head*)data)->type;
}

/*
static inline
int		lnet_get_event_fd(void* data){
	return ((check_head*)data)->fd;
}

static inline
int		lnet_get_event_type(uint64_t data){
	//return ((check_head*)&data)->type;
	return (int)(((data & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF);
}

static inline
int		lnet_get_event_fd(uint64_t data){
	//return ((check_head*)&data)->vfd;
	return (int)( data & 0xFFFFFFFF );
}
*/

static inline
int		lnet_gen_vfd(){
	spin_lock(&pnif->vfd_lock);
	int vfd = ++(pnif->g_vfd);
	spin_unlock(&pnif->vfd_lock);

	return vfd;
}

/*
static inline
uint64_t	lnet_make_eventdata(int event_type, int vfd)
{
	uint64_t event_data = 0;
	event_data |= ((uint64_t)event_type << 32) & 0xFFFFFFFF00000000;
	event_data |= ((uint64_t)vfd) & 0xFFFFFFFF;

	return event_data;
}
*/

inline
net_buff* lnet_get_nbuff(int vfd){
	return (net_buff*)hash_get_int(pnif->all_conn, vfd);
}

static inline
void	lnet_add_nbuff(int vfd, net_buff* nbuff){
	hash_set_int(pnif->all_conn, vfd, nbuff);
}

static inline
void	lnet_del_nbuff(int vfd){
	hash_del_int(pnif->all_conn, vfd);
}

static inline
int		lnet_isclose(int vfd){
	net_buff* nbuff = lnet_get_nbuff(vfd);
	if( !nbuff ) return 1;
	return 0;
}

/*
static inline
void	lnet_inc_ref(net_buff* nbuff){
	spin_lock(&pnif->ref_lock);
	nbuff->ref_count++;
	spin_unlock(&pnif->ref_lock);
}

static inline
void	lnet_dec_ref(net_buff* nbuff){
	spin_lock(&pnif->ref_lock);
	nbuff->ref_count--;
	spin_unlock(&pnif->ref_lock);
}
*/

static
void	check_net_arg(on_recv func_recv,
				on_error func_error)
{
	if ( !func_error )
	{
		printf("no callback func: on_error\n");
	}

	if ( !func_recv )
	{
		printf("no callback func: on_recv\n");
	}
}

static
int		_check_reg(int port, on_accept func_accept)
{
	if( !func_accept )
		return 1;

	liter iter = list_iter(pnif->plisten_list);

	listen_info* linfo;
	while( ( linfo = (listen_info*)list_each(&iter) ) )
	{
		if ( port == linfo->port )
			return 1;
	}

	return 0;
}

void	lnet_init()
{
	pnif = (nif*)malloc(sizeof(nif));
	if( !pnif ){
		printf("fatal error in lnet_init:could not create libnet\n");
		exit(0);
	}

	pnif->r_epfd = net_epoll_create(MAXPOLLNUM);
	pnif->g_vfd = 0;
	pnif->plisten_list = create_list();
	pnif->all_conn = hash_create(DEFAULT_CONN_HASH);
	pnif->ready2destroy = hash_create(DEFAULT_CONN_HASH);
	pnif->ev_pool = hash_create(DEFAULT_CONN_HASH * 10);
	pnif->recv_buf_size = NET_BUFF_RECV_SIZE;
	pnif->recv_buf = (char*)malloc(sizeof(char) * pnif->recv_buf_size);
	pnif->write_buf = (char*)malloc(sizeof(char) * NET_BUFF_SEND_SIZE);
	pnif->pshut_down_func = NULL;
	pnif->ready2destroy_timer = NULL;

	LOCK_INIT(&pnif->r_lock);
	LOCK_INIT(&pnif->w_lock);
	LOCK_INIT(&pnif->g_lock);
	cond_init(&pnif->r_cond);
	spin_init(&pnif->ref_lock);
	spin_init(&pnif->vfd_lock);

#ifdef _USE_SPIN_LOCK_
	printf("use spin lock\n");
#else
	printf("use mutex lock\n");
#endif
}

inline
void	lnet_set_global_recvsize(size_t size){
	pnif->recv_buf_size = size;
}

void	lnet_set_shutdown(on_shutdown_conn pfunc)
{
	pnif->pshut_down_func = pfunc;
}

void	lnet_push_queue(f_hash* phash, int fd, void* data)
{
	DOLOCK(&pnif->r_lock);
	void* src_data = hash_get_int(phash, fd);
	if( src_data ){
		UNLOCK(&pnif->r_lock);
		return;
	}

	hash_set_int(phash, fd, data);
	UNLOCK(&pnif->r_lock);
}

void	lnet_pop_queue(f_hash* phash, int fd)
{
	DOLOCK(&pnif->r_lock);
	hash_del_int(phash, fd);
	UNLOCK(&pnif->r_lock);
}

void	lnet_push_timer(f_hash* phash, int fd, void* data)
{
	hash_set_int(phash, fd, data);
}

void	lnet_pop_timer(f_hash* phash, int fd)
{
	hash_del_int(phash, fd);
}

int		lnet_register(int port, on_accept func_accept)
{
	if( _check_reg(port, func_accept) )
	{
		printf("lnet_register failed\n");
		return 1;
	}
	
	listen_info* linfo = (listen_info*)malloc(sizeof(listen_info));

	lnet_set_event_type(linfo, LNET_EVENT_TYPE_LISTEN);
	linfo->port = port;
	linfo->listen_fd = -1;
	linfo->is_block = FALSE;

	linfo->accept = func_accept;
	push_node(pnif->plisten_list, linfo);

	return 0;
}

net_buff* lnet_buff_new(int fd,
				on_recv func_recv,
				on_send func_send,
				on_error func_error, 
				void* u_data_ptr)
{
	check_net_arg(func_recv, func_error);
	net_set_nonblocking(fd);

	net_buff* data = (net_buff*)malloc(sizeof(net_buff));

	data->vfd = lnet_gen_vfd();
	lnet_set_event_type(data, LNET_EVENT_TYPE_BUFF);
	data->fd = fd;
	data->w_buff_size = NET_BUFF_SEND_SIZE;
	data->r_buff_size = NET_BUFF_RECV_SIZE;
	data->ptr = u_data_ptr;
	data->ref_count = 0;
	data->event = LNET_BUFF_EVENT_READ;
	data->ready2destroy = 0;

	data->r_active = 0;
	data->w_active = 0;

	data->recv = func_recv;
	data->send = func_send;
	data->error = func_error;

	lnet_add_nbuff(data->vfd, data);

	return data;
}

void	lnet_buff_del(net_buff* nbuff)
{
	lnet_try_close(nbuff);
}

//According to r_active and w_active value to set fd status in epoll
void	lnet_epoll_mod(int fd, net_buff* nbuff)
{
	int opt = 0;
	if( nbuff->r_active )
		opt |= FREAD;

	if( nbuff->w_active )
		opt |= FWRITE;

	net_epoll_mod(pnif->r_epfd, fd, opt, nbuff);
}

void	lnet_buff_start(net_buff* nbuff){
	nbuff->w_buff = nbuff->w_buff_size ? create_mbuf(nbuff->w_buff_size) : NULL;
	nbuff->r_buff = nbuff->r_buff_size ? create_mbuf(nbuff->r_buff_size) : NULL;
	assert( nbuff->w_buff );
	assert( nbuff->r_buff );
	nbuff->ref_count += 1;

	nbuff->r_active = 1;	//first read set active
	nbuff->ready2destroy = 0;

	net_epoll_add(pnif->r_epfd, lnet_get_fd(nbuff), FREAD, nbuff);
}

inline
void	lnet_set_wbuff_size(net_buff* nbuff, size_t size){
	nbuff->w_buff_size = size;
}

inline
void	lnet_set_rbuff_size(net_buff* nbuff, size_t size){
	nbuff->r_buff_size = size;
}

void	lnet_buff_real_del(net_buff* data)
{
	delete_mbuf(data->w_buff);
	delete_mbuf(data->r_buff);

	free(data);
}

etimer*	lnet_create_timer(on_timer pfunc, void* arg){
	if( !pfunc ) return NULL;

	etimer* et = (etimer*)malloc( sizeof(etimer) );
	et->pfunc = pfunc;
	et->arg = arg;
	
	int fd = ftimerfd_create();
	if ( fd == -1 )
		return NULL;

	et->fd = fd;
	lnet_set_event_type(et, LNET_EVENT_TYPE_TIMER);

	return et;
}

int		lnet_start_timer(etimer* et, long long first, long long alter){
	if( !ftimerfd_start(et->fd, first, alter) ){
		DOLOCK(&pnif->g_lock);

		net_epoll_add(pnif->r_epfd, et->fd, FREAD, et);

		UNLOCK(&pnif->g_lock);
		return 0;
	}
	
	return 1;
}

int		lnet_stop_timer(etimer* et){
	if( !ftimerfd_stop(et->fd) ){
		DOLOCK(&pnif->g_lock);

		net_epoll_del(pnif->r_epfd, et->fd);
		close(et->fd);
		free(et);

		UNLOCK(&pnif->g_lock);
		return 0;
	}

	return 1;
}

static
void	lnet_init_listen()
{
	liter iter = list_iter(pnif->plisten_list);

	listen_info* linfo;
	while( ( linfo = (listen_info*)list_each(&iter) ) )
	{
		int listen_fd = linfo->listen_fd;

		net_epoll_add(pnif->r_epfd, listen_fd, FREAD, linfo);
	}
}

static
void	lnet_on_accept(listen_info* linfo)
{
	while(1){
		int new_fd = net_accept(linfo->listen_fd);	
		printf("can accept listen fd = %d\n", linfo->listen_fd);

		if( new_fd > 0 )
			linfo->accept(new_fd);
		else
			break;
	}
}

int		lnet_send_nocache(net_buff* nbuff, char* buff, size_t len){
	return net_send(nbuff->fd, buff, len);
}

int		lnet_send(net_buff* nbuff, const char* buff, size_t len)
{
	int send_num = 0, fd, buf_len;
	mbuf* wbuf = NULL;

	wbuf = nbuff->w_buff;
	fd = lnet_get_fd(nbuff);
	buf_len = mbuf_used(wbuf);
	if( buf_len > 0 ){
		// push all into wbuf
		if( mbuf_push(wbuf, buff, len) ){
			printf("1w_buff no space!!, fd=%d buff_size=%d\n", fd, mbuf_used(wbuf));
			lnet_close(nbuff);
		}
		else{
			if( nbuff->w_active == 0 ){
				nbuff->w_active = 1;
				lnet_epoll_mod(fd, nbuff);
			}
		}
	}
	else{
		send_num = net_send(fd, buff, len);
		if( send_num > 0 && send_num < (int)len ){
			// push the left data into wbuf
			if( mbuf_push(wbuf, buff+send_num, len-send_num) ){
				printf("2w_buff no space!!, fd=%d buff_size=%d\n", fd, mbuf_used(wbuf));
				lnet_close(nbuff);
			}
			else{
				if( nbuff->w_active == 0 ){
					nbuff->w_active = 1;
					lnet_epoll_mod(fd, nbuff);
				}
			}
		}
		else if( send_num == SOCKET_ERROR ){
			lnet_close(nbuff);
		}
	}

	return send_num;
}

inline
void	lnet_close(net_buff* nbuff){
	int fd = lnet_get_fd(nbuff);

	nbuff->w_active = 0;
	shutdown(fd, SHUT_RDWR);	// only shutdown the read 
}

inline
void	lnet_safe_close(int fd)
{
	net_epoll_del(pnif->r_epfd, fd);
	net_close(fd);	// repeat close fd may be call epoll_wait...
}


static inline
void	lnet_clean(net_buff* nbuff){
	int fd = lnet_get_fd(nbuff);
	int vfd = lnet_get_vfd(nbuff);

	if( lnet_get_nbuff(vfd) ){
		printf("client quit fd=%d vfd=%d user_data=%p\n", fd, vfd, nbuff->ptr);

		lnet_safe_close(fd);
		lnet_buff_real_del(nbuff);
		lnet_del_nbuff(vfd);
	}
}

static
void	lnet_recv(net_buff* nbuff, char* buff, int len)
{
	int fd = lnet_get_fd(nbuff);
	void* user_data = nbuff->ptr;

	int free_len = lnet_get_freelen(nbuff, LNET_BUFF_READ);
	int fetch_len = free_len < len ? free_len : len;
	if( fetch_len == 0 ){
		printf("read buf has full fd=%d, free_len=%d fetch_len=%d\n", fd, free_len, fetch_len);
		//nbuff->recv(nbuff, user_data);
		lnet_proc_user_close(nbuff, TRUE);
		return;
	}

	int recv_size = net_recv(fd, buff, fetch_len);
	if( recv_size == SOCKET_IDLE || recv_size == SOCKET_BUSY )
		return;
	else if( recv_size == SOCKET_ERROR || recv_size == SOCKET_CLOSE ){
		printf("libnet: sock error fd=%d\n", fd);
		lnet_proc_user_close(nbuff, TRUE);
	}
	else
	{
		// call back func
		if( nbuff->recv )
		{
			mbuf_push(nbuff->r_buff, buff, recv_size);
			nbuff->recv(nbuff, user_data);
		}
	}
}

static
void	lnet_call_timer(etimer* data){
	etimer* et = data;
	
	uint64_t exp;	
	while(1){
		int read_size = read(et->fd, (char*)&exp, sizeof(uint64_t));
		if( read_size != sizeof(uint64_t) ){
			if( errno == EINTR )
				continue;
			return;
		}
		else
		{
			if( et->pfunc ){
				et->pfunc(et->arg);
			}
		}
	}
}

static inline
void	lnet_notice_user_error(net_buff* nbuff)
{
	if( !nbuff->ready2destroy ){
		nbuff->ready2destroy = 1;

		if( nbuff->error )
			nbuff->error(nbuff, lnet_get_arg(nbuff));
	}
}

static inline
void	lnet_real_close(net_buff* nbuff)
{
	lnet_clean(nbuff);
}

static 
void	lnet_proc_user_close(net_buff* nbuff, int add)
{
	int vfd = lnet_get_vfd(nbuff);
	//if( add ) lnet_push_timer(pnif->ready2destroy, vfd, nbuff);
	if( add ){
		printf("close by sock error\n");
	}
	else
		printf("close by timer queue\n");

	// ask logic whether can close
	if( pnif->pshut_down_func ){
		int ret = pnif->pshut_down_func(nbuff, lnet_get_arg(nbuff));
		
		if( ret == LNET_CAN_CLOSE ){
			lnet_notice_user_error(nbuff);

			lnet_pop_timer(pnif->ready2destroy, vfd);
		}
		else if( ret == LNET_FIX ){
			lnet_pop_timer(pnif->ready2destroy, vfd);
		}
	}
	else{
		printf("lnet notice error vfd=%d\n", vfd);
		lnet_notice_user_error(nbuff);

		//lnet_pop_timer(pnif->ready2destroy, vfd);
	}
}

void	lnet_try_close(net_buff* nbuff)
{
	int vfd = lnet_get_vfd(nbuff);
	int write_buf_usedlen = lnet_get_usedlen(nbuff, LNET_BUFF_WRITE);

	printf("try close: fd=%d vfd=%d ref_count=%d r_active=%d w_active=%d write_buflen=%d\n",
			lnet_get_fd(nbuff), vfd, nbuff->ref_count, 
			nbuff->r_active, nbuff->w_active,
			write_buf_usedlen);

	if( /*nbuff->ref_count <= 0 && */
		/*nbuff->read_destroy ==*/ 1 ){

		if( !lnet_isclose(vfd) ){
			lnet_real_close(nbuff);
		}
	}
	else{
		printf("libnet warning: call lnet_buff_del must call lnet_close() or network error\n");
	}
}

static
int		lnet_delete_foreach(void* data)
{
	net_buff* nbuff = (net_buff*)data;

	lnet_proc_user_close(nbuff, FALSE);
	return 0;
}

static 	
void 	lnet_check_delete_timer(void* arg){
	//printf("delete timer run...\n");
	//printf("conn hash table size=%d\n", hash_get_count(pnif->r_pool));
	//hash_statistics(pnif->r_pool);
	if( hash_get_count(pnif->ready2destroy) )
		hash_foreach(pnif->ready2destroy, lnet_delete_foreach);
}

static
void	lnet_send_left(net_buff* nbuff, char* buff)
{
	while( 1 ){
		int buff_size = mbuf_used(nbuff->w_buff);

		if ( buff_size > 0 )
		{
			int off = buff_size > NET_BUFF_SEND_SIZE ? NET_BUFF_SEND_SIZE : buff_size;
			char* data = (char*)mbuf_getraw(nbuff->w_buff, buff, off);
			int send_num = net_send_safe(nbuff->fd, data, off);
			printf("send left:%d fd=%d realsend=%d\n", buff_size, nbuff->fd, send_num);
			if ( send_num > 0 )
				mbuf_tail_move(nbuff->w_buff, send_num);
			else{
				if( send_num == SOCKET_ERROR )
					lnet_close(nbuff);
				else
					break;
			}
		}
		else{
			nbuff->w_active = 0;
			lnet_epoll_mod(nbuff->fd, nbuff);
			break;
		}
	}
}

inline
void	lnet_proc_send(net_buff* nbuff, char* buff)
{
	if( nbuff->send )
	{
		int used = mbuf_used(nbuff->w_buff);
		mbuf_pop(nbuff->w_buff, buff, used);

		if( used > 0 )
			nbuff->send(nbuff, nbuff->ptr, used, buff);
		else{
			nbuff->w_active = 0;
			lnet_epoll_mod(nbuff->fd, nbuff);
		}
	}
	else
		lnet_send_left(nbuff, buff);
}

static inline
void	process_net_buff(struct epoll_event* ev, net_buff* nbuff)
{
	if( nbuff->w_active ){
		lnet_proc_send(nbuff, pnif->write_buf);
	}

	if ( nbuff->r_active ){
		lnet_recv(nbuff, pnif->recv_buf, pnif->recv_buf_size);
	}
}

static inline
void	process_timer(struct epoll_event* ev, etimer* et)
{
	lnet_call_timer(et);
}

static inline
void	process_connect(struct epoll_event* ev, conn_ev* cev)
{
	if( ev->events & (EPOLLHUP | EPOLLERR) ){
		printf("libnet: conn error fd=%d\n", cev->fd);
		cev->pc(cev->fd, LNET_CONN_ERROR, cev->arg);

		net_epoll_del(pnif->r_epfd, cev->fd);
		hash_del_int(pnif->ev_pool, cev->fd);
		close(cev->fd);
		free(cev);
	}
			
	if( ev->events & EPOLLOUT ){
		printf("libnet: conn sucess fd=%d\n", cev->fd);
		net_epoll_del(pnif->r_epfd, cev->fd);

		int error = 0;
		socklen_t len = sizeof(int);
		if (( 0 == getsockopt(cev->fd, SOL_SOCKET, SO_ERROR, &error, &len) )){
			if( 0 == error ){
				cev->pc(cev->fd, LNET_CONN_SUCESS, cev->arg);
			}
			else{
				printf("connect fd has not ready! fd=%d\n", cev->fd);
				cev->pc(cev->fd, LNET_CONN_ERROR, cev->arg);
				close(cev->fd);
			}
		}
		else{
			printf("connect fd getsockopt error! fd=%d\n", cev->fd);
			cev->pc(cev->fd, LNET_CONN_ERROR, cev->arg);
			close(cev->fd);
		}

		hash_del_int(pnif->ev_pool, cev->fd);
		free(cev);
	}
}

static inline
int		_connect_timeout(void* value)
{
	conn_ev* cev = (conn_ev*)value;
	int now = time(NULL);
	int time_out = cev->timeout;

	if( time_out >= 0 && ( (now - cev->conn_time) >= time_out ) ){
		printf("time out connect fd=%d\n", cev->fd);
		cev->pc(cev->fd, LNET_CONN_TIMEOUT, cev->arg);

		net_epoll_del(pnif->r_epfd, cev->fd);
		hash_del_int(pnif->ev_pool, cev->fd);
		close(cev->fd);
		free(cev);
	}

	return 0;
}

static inline
void	process_connect_timeout()
{
	if( hash_get_count(pnif->ev_pool) )
		hash_foreach(pnif->ev_pool, _connect_timeout);
}

static inline
void	process_accept(struct epoll_event* ev, listen_info* linfo)
{
	lnet_on_accept(linfo);
}

void	lnet_start_service()
{	
	int nfds = 0, i = 0, type;
	int epfd = pnif->r_epfd;
	int last_check_time = 0;
	void* data = NULL;
	struct epoll_event* events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_QUEUE_NUM);

	printf("start service\n");
	while(1)
	{
		last_check_time = time(NULL);
		nfds = epoll_wait(epfd, events, EPOLL_QUEUE_NUM, EPOLL_TIME_OUT);  

		for (i=0; i<nfds; ++i)
		{
			data = events[i].data.ptr;

			// ever EPOLLIN OR EPOLLOUT push into queue to process
			type = lnet_get_event_type(data);

			switch(type){
				case LNET_EVENT_TYPE_LISTEN:
					process_accept(&events[i], (listen_info*)data);
					break;
				case LNET_EVENT_TYPE_BUFF:
					process_net_buff(&events[i], (net_buff*)data);
					break;
				case LNET_EVENT_TYPE_TIMER:
					process_timer(&events[i], (etimer*)data);
					break;
				case LNET_EVENT_TYPE_CONN:
					process_connect(&events[i], (conn_ev*)data);
					break;

				default:
					printf("unknow type: %d\n", type);
			}
		}
		
		/*
		// time out
		if( nfds == 0 )
			printf("main thread time out nfds=%d\n", nfds);

		if( nfds == -1 )
			printf("main thread error %s\n", strerror(errno));
		*/

		if( time(NULL) - last_check_time >= LNET_CONN_CHECK_ALTER ){
			process_connect_timeout();
		}
	}

	free(events);
}

void	create_listen()
{
	liter iter = list_iter(pnif->plisten_list);

	listen_info* data;
	while( (data = (listen_info*)list_each(&iter) ) )
	{
		int port = data->port;
		int is_block = data->is_block;

		int listen_fd = net_create_listen(NULL, port, LISTEN_QUEUE_NUM, is_block);

		if ( listen_fd < 0 )
		{
			printf("listen on port %d failed\n", port);
			exit(1);
		}

		data->listen_fd = listen_fd;
		net_set_nonblocking(data->listen_fd);
		printf("listen_port = %d fd=%d\n", port, data->listen_fd);
	}
}

void	create_ready_delete_timer()
{
	pnif->ready2destroy_timer = lnet_create_timer(lnet_check_delete_timer, NULL);
	if( !pnif->ready2destroy_timer ){
		printf("create ready to delete queue failed\n");
		exit(1);
	}

	long alter = 1000 * 1000000l * 2;	// 2 seconds
	lnet_start_timer(pnif->ready2destroy_timer, alter, alter);
	printf("start delete timer\n");
}

void	lnet_dispatch()
{
	create_listen();
	lnet_init_listen();
	create_ready_delete_timer();

	// start recv
	lnet_start_service();
}

int		lnet_conn(const char* ip, int port, int isblock)
{
	return net_conn(ip, port, isblock);
}

// async connect
// return: 
// > 0 : sucess connect , you can use it
// -1 : error
// 0 : connect has in process
//
// arg:
// timeout: >= 0 as normal, < 0 infinite
int		lnet_conn_a(const char* ip, int port, int is_block, int timeout, pconn pfunc, conn_arg_t arg)
{ 
	int sockfd = -1;
	int s = net_conn_a(ip, port, &sockfd);

	if( s == 0 ){	// connect sucess
		return sockfd;
	}
	else if( s == -1 ){ // connect error
		return -1;
	}
	else{ // connect has in process 
		conn_ev* cev = (conn_ev*)malloc(sizeof(conn_ev));
		lnet_set_event_type(cev, LNET_EVENT_TYPE_CONN);
		cev->fd = sockfd;
		cev->timeout = timeout;
		cev->is_block = is_block;
		cev->conn_time = time(NULL);
		cev->pc = pfunc;
		cev->arg.u64 = arg.u64;	//copy from arg

		net_epoll_add(pnif->r_epfd, sockfd, FREAD | FWRITE, cev);
		hash_set_int(pnif->ev_pool, cev->fd, cev);
		return 0;
	}
}

inline	
int 	lnet_get_fd(net_buff* nbuff)
{
	return nbuff->fd;
}

inline
int		lnet_get_vfd(net_buff* nbuff)
{
	return nbuff->vfd;
}

inline
int		lnet_get_ip(net_buff* nbuff)
{
	return nbuff->ip;
}

inline
int		lnet_get_port(net_buff* nbuff)
{
	return nbuff->port;
}

inline
int		lnet_get_bufflen(net_buff* nbuff){
	return mbuf_used(nbuff->r_buff);
}

inline
void*	lnet_get_buffdata(net_buff* nbuff, void* pbuf, size_t len){
	return mbuf_vpop(nbuff->r_buff, pbuf, len);
}

inline
void*	lnet_get_buffraw(net_buff* nbuff, void* buff, size_t len, int type){
	mbuf* pbuf = NULL;
	if( type == LNET_BUFF_READ )
		pbuf = nbuff->r_buff;
	else if( type == LNET_BUFF_WRITE )
		pbuf = nbuff->w_buff;
	else
		return NULL;

	return mbuf_getraw(pbuf, buff, len);
}

inline
void	lnet_cut_buff(net_buff* nbuff, size_t len, int type){
	if( type == LNET_BUFF_READ )
		mbuf_tail_move(nbuff->r_buff, len);
	else if( type == LNET_BUFF_WRITE )
		mbuf_tail_move(nbuff->w_buff, len);
	else
		return;
}

inline
int		lnet_get_freelen(net_buff* nbuff, int type){
	if( type == LNET_BUFF_READ )
		return mbuf_total_free(nbuff->r_buff);
	else
		return mbuf_total_free(nbuff->w_buff);
}

int		lnet_get_usedlen(net_buff* nbuff, int type){
	if( type == LNET_BUFF_READ )
		return mbuf_used(nbuff->r_buff);
	else
		return mbuf_used(nbuff->w_buff);
}

inline
void*	lnet_get_arg(net_buff* nbuff){
	return nbuff->ptr;
}

inline
void	lnet_set_arg(net_buff* nbuff, void* arg){
	nbuff->ptr = arg;
}

inline
void	lnet_foreach(lnet_each_cb pfunc){
	int _foreach(void* data){
		if(pfunc)
			pfunc((net_buff*)data);

		return 0;
	}

	hash_foreach(pnif->all_conn, _foreach);
}

inline
void	lnet_set_sys_wbuff_size(net_buff* nbuff, size_t size){
	net_set_send_buffsize(lnet_get_fd(nbuff), size);
}

inline
void	lnet_set_sys_rbuff_size(net_buff* nbuff, size_t size){
	net_set_recv_buffsize(lnet_get_fd(nbuff), size);
}

inline
void	lnet_set_nodely(net_buff* nbuff){
	net_set_nodely(lnet_get_fd(nbuff));
}

int		lnet_buff_count()
{
	return hash_get_count(pnif->all_conn);
}

/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/

