#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "../base/mbuf.h"
#include "../base/net_core.h"
#include "../module/pto/pto_proc.h"
#include "tu_inc.h"

#define TEST_HOST	"www.baidu.com"
#define BUFF_SIZE	1024 * 1024

static
void	pack_complete_cb(char* pbuf, int len, void* arg){
	int fd = *(int*)arg;
	net_send_safe(fd, pbuf, len);
}

static
int		do_recv(mbuf* g_buf, int fd, char* pbuf, int len){
	assert( !mbuf_push(g_buf, pbuf, len) );
	char tmp[100];

	do{
		if( mbuf_used(g_buf) < 2 )
			return 1;

		char* ret = mbuf_getraw(g_buf, tmp, 2);
		int tl;
		int pto_len = pto_get_len(ret, 100, &tl);

		int buf_len = mbuf_used(g_buf);
		if( buf_len >= pto_len + 2 ){
			assert( !mbuf_pop(g_buf, tmp, pto_len + 2) );

			pto_unpack(tmp + 2, pto_len, NULL);

			return 0;
		}
		else
			return 1;

	}while(1);
}


static
void*	run(void* arg)
{
	int fd = *(int*)arg;
	mbuf* g_buf = create_mbuf(65536);
	char* buff = (char*)malloc(BUFF_SIZE);

	while(1)
	{
		int ret = recv(fd, buff, BUFF_SIZE, MSG_NOSIGNAL);

		if( ret < 0 )
		{
			printf("recv error errno=%d(%s)\n", errno, strerror(errno));
			if( errno == EINTR )
				continue;

			return NULL;
		}
		else if( ret == 0 )
		{
			printf("server gone away\n");
			return NULL;
		}
		else
		{
			do_recv(g_buf, fd, buff, ret);
		}
	}

	return NULL;
}

static
void	host_resolve(pto_data* pdata, void* arg)
{
	char* uid, *ips;
	pto_get_strdata(pdata, 1, &uid);
	pto_get_strdata(pdata, 2, &ips);

	printf("host resolve uid=%s ips=%s\n", uid, ips);
}

static
void	t_unpack_cm(pto_data* pdata, void* arg){
	pto_run(pdata, arg);
	pto_data_free(pdata);
}

void	test_dns()
{
	pto_init(pack_complete_cb, t_unpack_cm, NULL, NULL);
	pto_load();
	pto_register("s_host_resolve", host_resolve);

	int fd = net_conn("127.0.0.1", 11280, 1);
	if( fd == -1 ){
		printf("cannot connect dns service\n");
		exit(1);
	}

	pthread_t tid;
	pthread_create(&tid, 0, run, &fd);

	int i = 0;
	for(; ; ++i){
		PTO_PACK("d_host_resolve", &fd, "12345", TEST_HOST);
		usleep(100);
	}

	pthread_join(tid, NULL);
}
