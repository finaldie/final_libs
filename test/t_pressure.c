//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "../base/libnet.h"
#include "../base/net_core.h"
#include "../module/pto/pto_proc.h"
#include "../base/mbuf.h"
#include "../base/ltimer.h"
#include "tu_inc.h"

#define GIP 	"192.168.7.136"
#define	PORT	5118

#define BUFF_SIZE 	32768
#define STAT_TIMEOUT	(1000000000l)

//TODO...
static int g_num = 0;

static
void	pack_complete_cb(char* pbuf, int len, void* arg){
	int fd = *(int*)arg;

	assert( *(unsigned short*)pbuf == 7 );
	assert( *(unsigned short*)(pbuf + 2) == 0 );
	assert( len == 9 );

	int total_len = 0;

	while(total_len < len)
	{
		int s_len = send(fd, pbuf+total_len, len-total_len, MSG_NOSIGNAL);

		if( s_len != 9 ){
			printf("client send != 9, sendlen=%d\n", s_len);

			if( s_len == -1 ){
				printf("send error errno=%d reason=%s\n", errno, strerror(errno));

				if( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ){
					continue;
				}
				else
					exit(1);
			}
			else
				total_len += s_len;

		}
		else
			total_len += s_len;
	};
}

int		do_recv(mbuf* g_buf, int fd, char* pbuf, int len){
	assert( !mbuf_push(g_buf, pbuf, len) );
	char tmp[100];

	do{
		if( mbuf_used(g_buf) < 2 )
			return 1;

		char* ret = mbuf_getraw(g_buf, tmp, 2);
		int tl;
		int pto_len = pto_get_len(ret, 100, &tl);
		//printf("gy: unpack len = %d offset=%d\n", pto_len, tl);
		assert(pto_len == 7);

		int buf_len = mbuf_used(g_buf);
		//printf("gbuf_len = %d\n", buf_len);
		if( buf_len >= pto_len + 2 ){
			assert( !mbuf_pop(g_buf, tmp, pto_len + 2) );

			pto_unpack(tmp + 2, pto_len, NULL);

			return 0;
		}
		else
			return 1;

	}while(1);
}

void	run(void* arg){
	mbuf* g_buf = NULL;

	g_buf = create_mbuf(65536);
	int fd = lnet_conn(GIP, PORT, 1);

	PTO_PACK("s_show", &fd, 1);

	char* buff = (char*)malloc(BUFF_SIZE);

	while(1)
	{
		int ret = recv(fd, buff, BUFF_SIZE, MSG_NOSIGNAL);

		if( ret < 0 )
		{
			printf("recv error errno=%d(%s)\n", errno, strerror(errno));
			if( errno == EINTR )
				continue;

			return;
		}
		else if( ret == 0 )
		{
			printf("server gone away\n");
			return;
		}
		else
		{
			do_recv(g_buf, fd, buff, ret);
			return;
		}
	}
}

void	show_num(pto_data* pdata, void* arg){
	int num;
	pto_get_intdata(pdata, 1, &num);
	
	printf("s_show num:%d\n", num);
	g_num = num;
}

void	t_unpack_cm(pto_data* pdata, void* arg){
	pto_run(pdata, arg);
	pto_data_free(pdata);
}

void test_pressure(int argc, char** argv)
{
	pto_init(pack_complete_cb, t_unpack_cm, NULL, NULL);
	pto_load();
	pto_register("c_show", show_num);

	int max = atoi(argv[0]);
	printf("argc = %d\n", argc);
	if( argc < 1 ) return;
	printf("max = %d\n", max);

	pthread_t tid[max];
	int i;
	for( i=0; i< max; ++i ){
		pthread_create(&tid[i], 0, (void*)run, NULL);
	}

	for( i=0; i< max; ++i ){
		pthread_join(tid[i], NULL);
	}

	return;
}

