//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "ltimer.h"

//TODO...
static pthread_once_t init_catch = PTHREAD_ONCE_INIT;

struct gt_catch{
	struct sigaction sa;
	sigset_t 	mask;
}g_catch;

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN
#define TRANS_STONS	1000000000

#define errExit(msg)    do {	\
	 perror(msg); exit(EXIT_FAILURE); \
	} while (0)

void handler(int sig, siginfo_t *si, void *uc){
	if( sig != SIGRTMIN ) return;

	f_timer* pt = (f_timer*)si->si_value.sival_ptr; 
	if( pt->pfunc )
		pt->pfunc(pt->arg);

	//signal(sig, SIG_IGN);
}

static 
void ftimer_create_signal(){
    g_catch.sa.sa_flags = SA_SIGINFO;
    g_catch.sa.sa_sigaction = handler;
    //sigemptyset(&g_catch.sa.sa_mask);
    if (sigaction(SIG, &g_catch.sa, NULL) == -1){
        exit(0);
    }

    //sigemptyset(&g_catch.mask);
    //sigaddset(&g_catch.mask, SIG);
    //if (sigprocmask(SIG_SETMASK, &g_catch.mask, NULL) == -1)
    //	return 2;
}

int	ftimer_create(f_timer* pt, long long nsecs, long long alter, ptimer pfunc, void* arg){
    pthread_once(init_catch, ftimer_create_signal);

	pt->sev.sigev_notify = SIGEV_SIGNAL;
	pt->sev.sigev_signo = SIG;
	pt->sev.sigev_value.sival_ptr = pt;	//store self
	if (timer_create(CLOCKID, &pt->sev, &pt->timerid) == -1)
		return 1;

	pt->its.it_value.tv_sec = nsecs / TRANS_STONS;
	pt->its.it_value.tv_nsec = nsecs % TRANS_STONS;
	pt->its.it_interval.tv_sec = alter / TRANS_STONS;
	pt->its.it_interval.tv_nsec = alter % TRANS_STONS;

	pt->pfunc = pfunc;
	pt->arg = arg;

	return 0;
}

inline
int ftimer_start(f_timer* pt){
	if (timer_settime(pt->timerid, 0, &pt->its, NULL) == -1)
		return 1;
	return 0;
}

inline
int ftimer_del(f_timer* pt){
	return timer_delete(pt->timerid);
}

int	ftimerfd_create(){
	int fd = timerfd_create(CLOCKID, TFD_NONBLOCK);

	if( fd == -1 )
		return -1;
	return fd;
}

int ftimerfd_start(int fd, long long nsesc, long long alter){
	struct itimerspec new_value;
	new_value.it_value.tv_sec = nsesc / TRANS_STONS;
	new_value.it_value.tv_nsec = nsesc % TRANS_STONS;
	new_value.it_interval.tv_sec = alter / TRANS_STONS;
	new_value.it_interval.tv_nsec = alter % TRANS_STONS;

	if( timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1 )
		return 1;
	return 0;
}

int ftimerfd_stop(int fd){
	struct itimerspec new_value;
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_nsec = 0;
	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 0;

	if( timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1 )
		return 1;
	return 0;
}

/*
// gcc -Wall -g -lrt -lpthread -o time ltimer.c
int	main(int argc, char** argv)
{
	printf("main tid=%lu\n", pthread_self());
	void test_s(void* arg){
		printf("aaa %d\n", *(int*)arg);
	}

	f_timer ti;
	int i = 1;
	if( ftimer_create(&ti, 1000000000, 500000000, test_s, &i) )
		printf("create error\n");

	if( ftimer_start(&ti) )
		printf("start error\n");

	while(1){
		sleep(10);
	}
	return 0;
}
*/
