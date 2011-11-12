//base info: create by hyz
//effect:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "laio_write.h"

//TODO...

// the write complete call back run on another thread
void	aio_write_complete( sigval_t sigval ){
	laio_w* req = (laio_w*)sigval.sival_ptr; 
	if (aio_error( &req->_acb ) == 0) {
		int ret = aio_return( &req->_acb ); 
		if( req->pwc && req->pwc(req->data, ret) ){}
	} 
	
	return; 
}

void	_laio_write(laio_w* w_acb, int fd, void* buff, int len){
	memset(&w_acb->_acb, 0, sizeof(acb));

	w_acb->_acb.aio_fildes = fd; 
	w_acb->_acb.aio_buf = buff; 
	w_acb->_acb.aio_nbytes = len; 
	w_acb->_acb.aio_offset = 0; 

	w_acb->_acb.aio_sigevent.sigev_notify = SIGEV_THREAD; 
	w_acb->_acb.aio_sigevent.sigev_notify_function = aio_write_complete; 
	w_acb->_acb.aio_sigevent.sigev_notify_attributes = NULL; 
	w_acb->_acb.aio_sigevent.sigev_value.sival_ptr = w_acb; 

	aio_write(&w_acb->_acb);
}

inline
void	laio_init(laio_w* w_acb, pfunc_wc pfunc, void* data){
	w_acb->pwc = pfunc;
	w_acb->data = data;
}

inline
void	laio_write(laio_w* w_acb, int fd, void* buff, int len){
	_laio_write(w_acb, fd, buff, len);
}

/*
#define LOOP_NUM 1000
// aio module need lrt lib
// gcc -Wall -g -lrt -o aio_t laio_write.c
int	main(int argc, char** argv)
{
	int test_wc(int len){
		//printf("write len=%d\n", len);
		return 0;
	}

	laio_w w_acb[LOOP_NUM];
	printf("main tid=%lu\n", pthread_self());
	int i;
	char info[LOOP_NUM][40];
	for(i=0; i<LOOP_NUM; ++i){
		sprintf(info[i], "test%d\n", i);
		laio_write(&w_acb[i], "./test.log", info[i], strlen(info[i]), test_wc);
	}

	printf("sleep...\n");
	sleep(10);
	return 0;
}
*/
