#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tu_inc.h"
#include "../base/ltimer.h"

#define TKEYTIMEOUT	(1000000000l)

static
pthread_key_t key;

static
f_timer ti;

static
void	th_timer(void* arg){
	printf("timer\n");
}

static
void	th_des(void* arg){
	printf("des arg addr = %p %d\n", arg, *(int*)arg);
	f_timer* t = (f_timer*)arg;
	ftimer_del(t);
}

static
void	run(void* arg){
	int * n = (int*)malloc(4);
	*n = 101;
	printf("ti addr = %p\n", &ti);
	ftimer_create(&ti, TKEYTIMEOUT, TKEYTIMEOUT, th_timer, NULL);
	pthread_setspecific(key, &ti);
}

void	test_th_key(){
	printf("key addr = %p\n", &key);
	pthread_key_create(&key, th_des);

	pthread_t tid;
	pthread_create(&tid, 0, (void*)run, NULL);

	pthread_join(tid, NULL);
}
