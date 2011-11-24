#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tu_inc.h"
#include "lmempool.h"
#include "inc.h"

//#define malloc 	f_alloc
//#define free 	f_free

#pragma pack(4)
typedef struct{
	int idx:8;
	unsigned int alloc_len:24;		// for optimization realloc
}fb_head;

typedef struct _fb{
	fb_head		b_head;

	union {
		char		block[0];
		struct _fb*	next;	 
	}block_data;

}free_block;
#pragma pack()

#define LOOP_NUM 	1000000
#define THREAD_NUM	2
//static void** a_ptr;

void	alloc_test(int size, int idx){
	my_time t1;
	get_cur_time(&t1);

	int i;
	for( i=0; i<LOOP_NUM; ++i ){
		int* a_ptr = (int*)f_alloc(size);
        free_block* fb = (free_block*)((fb_head*)a_ptr - 1);

        FTU_ASSERT_EQUAL_INT(idx, fb->b_head.idx);
		if( idx != -1 )
        	FTU_ASSERT_EQUAL_INT(size, fb->b_head.alloc_len);

		*(int*)a_ptr = 10;
		f_free(a_ptr);
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

	get_cur_time(&t1);
	alloc_test(4, 0);
	alloc_test(12, 1);
	alloc_test(20, 2);
	alloc_test(30, 3);
	alloc_test(36, 4);
	alloc_test(60, 5);
	alloc_test(100, 6);
	alloc_test(180, 7);
	alloc_test(300, 8);
	alloc_test(600, 9);
	alloc_test(1000, 10);
	alloc_test(1200, 11);
	alloc_test(3000, 12);
	alloc_test(5000, 13);
	alloc_test(1024*4*2+10, 14);
	alloc_test(1024*4*4+10, 15);
	alloc_test(1024*50, -1);

	get_cur_time(&t2);
	int di = get_diff_time(&t1, &t2);
	printf("tid=%lu total diff_time:%dusec | %dms avg=%dms\n", pthread_self(), di, di/1000, (di/1000)/17);

	*(int*)arg = di/1000;
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
}

void*	realloc_test(void* arg){
	int* n = (int*)f_alloc(4);
    free_block* fb = (free_block*)((fb_head*)n - 1);
    FTU_ASSERT_EQUAL_INT(0, fb->b_head.idx);
    FTU_ASSERT_EQUAL_INT(4, fb->b_head.alloc_len);

	int* r = f_realloc(n, 16);
    fb = (free_block*)((fb_head*)r - 1);
    FTU_ASSERT_EQUAL_INT(1, fb->b_head.idx);
    FTU_ASSERT_EQUAL_INT(16, fb->b_head.alloc_len);

	f_free(r);

	return NULL;
}

void	test_realloc(){
	pthread_t tid[THREAD_NUM];
	int i;
	for(i=0; i<THREAD_NUM; ++i){
		pthread_create(&tid[i], 0, (void*)realloc_test, NULL);
	}

	for(i=0; i<THREAD_NUM; ++i)
		pthread_join(tid[i], NULL);
}
