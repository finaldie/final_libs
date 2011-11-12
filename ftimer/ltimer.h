//base info: create by hyz
/*effect: real time timer
 * support both signal & file descriptor
*/


#ifndef _F_TIMER_H_
#define _F_TIMER_H_ 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

typedef void (*ptimer)(void*);
typedef struct _f_timer{
	timer_t timerid;
	struct  sigevent sev;
	struct  itimerspec its;

	ptimer  pfunc;
	void*   arg;
}f_timer;

int	ftimer_create(f_timer*, long long nsecs, long long alter, 
					ptimer pfunc, void* arg);
int ftimer_start(f_timer*);
int	ftimer_del(f_timer*);

#ifdef __linux__
#include <sys/timerfd.h>

int	ftimerfd_create();
int	ftimerfd_start(int fd, long long nsecs, long long alter);
int	ftimerfd_stop(int fd);
#endif

#ifdef __cplusplus
}
#endif

#endif
