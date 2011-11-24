//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO...

#include "tu_inc.h"
#include "../module/pto/pto_proc.h"
#include "../base/mbuf.h"

void	pack_complete_cb(char* pbuf, int len, void* arg){
	int vfd = *(int*)arg;
	printf("pack_complete: vfd = %d pbuf = %s len=%d\n", vfd, pbuf, len);
	printf("%d %d %d %d\n", (int)(*(unsigned short*)pbuf), (int)(*(unsigned short*)(pbuf+2)), (int)(*(pbuf+4)), *(int*)(pbuf+5));
}

void	show(pto_data* pdata, void* arg){
	int vfd = *(int*)arg;
	int num;
	int status = pto_get_intdata(pdata, 1, &num);

	if( !status )
		printf("vfd = %d data = %d\n", vfd, num);
	else
		printf("get data failed\n");

	PTO_PACK("c_show", &vfd, 12);
}

void	t_pto(){
	printf("pto proc test\n");

	pto_init(pack_complete_cb, NULL, NULL, NULL);
	pto_load();

	pto_register("s_show", show);
	pto_check();

	char* data = (char*)malloc(100);
	*(unsigned short*)data = 5;		//len
	*(unsigned short*)(data+2) = 0;	//pto_id
	*(data + 4) = 2;
	*(int*)(data + 5) = 10;

	int tl;
	int len = pto_get_len(data, 100, &tl);
	printf("tpto: unpack len = %d offset=%d\n", len, tl);
	pto_unpack(data + tl, 98, NULL);
	//int fd = 11;
	//pto_run(pdata, &fd);
}

void	pack_cb(char* pbuf, int len, void* arg){
	assert(len == 9);
	mbuf* pm = (mbuf*)arg;
	char unpack_buf[100];
	assert( !mbuf_push(pm, pbuf, len) );
	unsigned short plen = 0;
	assert( !mbuf_pop(pm, &plen, 2) );

	int tl = 2;
	//int plen = pto_get_len(unpack_buf, len, &tl);
	printf("tpto: unpack len = %d offset=%d\n", (int)plen, tl);
	assert(plen == 7);
	assert( !mbuf_pop(pm, unpack_buf, plen) );
	pto_unpack(unpack_buf, 100, NULL);

	//int num;
	//int status = pto_get_intdata(pdata, 1, &num);
	//assert( status == 0 );
	//printf("num = %d\n", num);
}

void	t_pto_loop(){
	pto_init(pack_cb, NULL, NULL, NULL);
	pto_load();
	mbuf* pm = create_mbuf(32768);

	//pto_register("s_show", show);


	int i;
	for( i=0; ; ++i){
		printf("curr i =%d\n", i);
		PTO_PACK("c_show", pm, i);
	}
}

/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/
