#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tu_inc.h"
#include "../base/lmempool.h"

#define malloc 	f_alloc
#define free 	f_free

typedef struct{
	int idx:8;
	unsigned int alloc_len:24;		// for optimization realloc
}fb_head;

#pragma pack(4)
typedef struct _fb{
	fb_head		b_head;

	union {
		char		block[0];
		struct _fb*	next;	 
	}block_data;

}free_block;
#pragma pack()

#define LOOP_NUM 	1000000
#define THREAD_NUM	8
//static void** a_ptr;

void	alloc_test(int size){
	my_time t1;
	get_cur_time(&t1);

	int i;
	for( i=0; i<LOOP_NUM; ++i ){
		int* a_ptr = (int*)malloc(size);
		*(int*)a_ptr = 10;
		free(a_ptr);
	}

	my_time t2;
	get_cur_time(&t2);
	int di = get_diff_time(&t1, &t2);
	printf("tid=%lu size=%d diff_time:%dusec | %dms\n", pthread_self(), size, di, di/1000);
}

void	_test_mem(void* arg){
	printf("tid=%lu\n", pthread_self());
	//a_ptr = (void**)malloc(sizeof(void*) * LOOP_NUM);

	my_time t1;
	my_time t2;

LOOP:
	get_cur_time(&t1);
	alloc_test(4);
	alloc_test(12);
	alloc_test(20);
	alloc_test(30);
	alloc_test(36);
	alloc_test(60);
	alloc_test(100);
	alloc_test(180);
	alloc_test(300);
	alloc_test(600);
	alloc_test(1000);
	alloc_test(1200);
	alloc_test(3000);
	alloc_test(5000);
	alloc_test(1024*4*2+10);
	alloc_test(1024*4*4+10);
	alloc_test(1024*50);

	get_cur_time(&t2);
	int di = get_diff_time(&t1, &t2);
	printf("tid=%lu total diff_time:%dusec | %dms avg=%dms\n", pthread_self(), di, di/1000, (di/1000)/17);

	*(int*)arg = di/1000;

	sleep(8);
	goto LOOP;
}

void	test_mem(int argc, char** argv){
	printf("main tid=%lu\n", pthread_self());

	pthread_t tid[THREAD_NUM];
	int final_time[THREAD_NUM];
	int i;
	for(i=0; i<THREAD_NUM; ++i){
		pthread_create(&tid[i], 0, (void*)_test_mem, &final_time[i]);
	}
	
	for(i=0; i<THREAD_NUM; ++i)
		pthread_join(tid[i], NULL);

	while(1){
		sleep(10);
	}
}

void*	realloc_test(void* arg){
	int* n = (int*)arg;
	int* r = f_realloc(n, 16);
	f_free(r);

	return NULL;
}

void	test_realloc(){
	int* n = (int*)f_alloc(4);
	pthread_t tid[THREAD_NUM];
	int i;
	for(i=0; i<THREAD_NUM; ++i){
		pthread_create(&tid[i], 0, (void*)realloc_test, n);
	}

	for(i=0; i<THREAD_NUM; ++i)
		pthread_join(tid[i], NULL);
}
