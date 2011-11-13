//base info: create by hyz
/*effect: lib net 
* desc: network lib with balance scheduling
* something you must know before use this lib module:
* @ all method are not thread safe
* @ in this lib all socket is nonblock
* @ ok have a fun! :)
*/


#ifndef _LIB_NET_H_FINAL_
#define _LIB_NET_H_FINAL_

#include <stdint.h>

#define LNET_BUFF_READ		0
#define LNET_BUFF_WRITE 	1

#define LNET_NOREPLY		0	
#define LNET_REPLY			1

#define LNET_CANNOT_CLOSE 	0
#define LNET_CAN_CLOSE		1
#define LNET_FIX			2

typedef struct _net_buff net_buff;
typedef struct _event_timer etimer;

typedef void	(*on_accept)(int fd);
typedef int		(*on_recv)(net_buff*, void* user_data);
typedef void	(*on_send)(net_buff*, void* user_data, int left_num, char* buff);
typedef void	(*on_error)(net_buff*, void* user_data);
typedef void 	(*on_connect)(net_buff*);
typedef void	(*on_timer)();
typedef int		(*on_shutdown_conn)(net_buff*, void* user_data);

void	lnet_init();
void	lnet_set_global_recvsize(size_t size);
void	lnet_set_shutdown(on_shutdown_conn pfunc);

int		lnet_register(int port, on_accept func_accept);

net_buff*	lnet_buff_new(int fd,
				on_recv func_recv,		// call when has recv data
				on_send func_send,		// call when data buff has space
				on_error func_error,	// call when socket has error
				void* u_data_ptr);		// user data
void	lnet_buff_del(net_buff* nbuff);

void	lnet_set_wbuff_size(net_buff*, size_t size);
void	lnet_set_rbuff_size(net_buff*, size_t size);
void	lnet_set_sys_wbuff_size(net_buff*, size_t size);
void	lnet_set_sys_rbuff_size(net_buff*, size_t size);
void	lnet_set_nodely(net_buff*);
void	lnet_buff_start(net_buff*);
void	lnet_dispatch();

#define LNET_CONN_SUCESS	0
#define LNET_CONN_TIMEOUT	1
#define LNET_CONN_ERROR		2
typedef union{
	int		 u32;
	uint64_t u64;
	void*	 ptr;
}conn_arg_t;
typedef void (*pconn)(int fd, int ev, conn_arg_t arg);

int		lnet_conn(const char* ip, int port, int isblock);
// timeout: -1 is infinite wait
int		lnet_conn_a(const char* ip, int port, int is_block, int timeout, pconn pfunc, conn_arg_t arg);
int		lnet_send(net_buff*, const char* buff, size_t len);
int		lnet_send_nocache(net_buff*, char* buff, size_t len);
void	lnet_close(net_buff*);

etimer* lnet_create_timer(on_timer pfunc, void* arg);
int		lnet_start_timer(etimer*, long long first, long long alter);
int		lnet_stop_timer(etimer*);

int		lnet_get_fd(net_buff*);
int		lnet_get_vfd(net_buff*);
void*	lnet_get_arg(net_buff*);
void	lnet_set_arg(net_buff*, void*);
int		lnet_get_ip(net_buff*);
int		lnet_get_port(net_buff*);

// get read buf len
int		lnet_get_bufflen(net_buff*);
int		lnet_get_usedlen(net_buff*, int type);
void*	lnet_get_buffdata(net_buff*, void* pbuf, size_t len);
void*	lnet_get_buffraw(net_buff*, void* pbuf, size_t len, int type);
void	lnet_cut_buff(net_buff*, size_t len, int type);
net_buff* lnet_get_nbuff(int vfd);

typedef void (*lnet_each_cb)(net_buff*);
void	lnet_foreach(lnet_each_cb);
int		lnet_buff_count();

#endif

