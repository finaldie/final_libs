#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "flibs/ftime.h"

static pthread_once_t init_catch = PTHREAD_ONCE_INIT;

struct gt_catch {
    struct sigaction sa;
    sigset_t         mask;
} g_catch;

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN
#define TRANS_STONS    1000000000l

#define errExit(msg) \
do { \
    perror(msg); exit(EXIT_FAILURE); \
} while (0)

void handler(int sig, siginfo_t *si, void *uc __attribute__((unused)))
{
    if( sig != SIGRTMIN ) return;

    ftimer* pt = (ftimer*)si->si_value.sival_ptr;
    if (pt->cb)
        pt->cb(pt->arg);
}

static
void ftimer_create_signal()
{
    g_catch.sa.sa_flags = SA_SIGINFO;
    g_catch.sa.sa_sigaction = handler;

    if (sigaction(SIG, &g_catch.sa, NULL) == -1){
        exit(0);
    }
}

int ftimer_create(ftimer* pt, long long nsecs, long long alter,
                    ftimer_cb pfunc, void* arg)
{
    pthread_once(&init_catch, ftimer_create_signal);

    pt->sev.sigev_notify = SIGEV_SIGNAL;
    pt->sev.sigev_signo = SIG;
    pt->sev.sigev_value.sival_ptr = pt;    //store self
    if (timer_create(CLOCKID, &pt->sev, &pt->timerid) == -1)
        return 1;

    pt->its.it_value.tv_sec = (time_t)(nsecs / TRANS_STONS);
    pt->its.it_value.tv_nsec = (long int)(nsecs % TRANS_STONS);
    pt->its.it_interval.tv_sec = (time_t)(alter / TRANS_STONS);
    pt->its.it_interval.tv_nsec = (long int)(alter % TRANS_STONS);

    pt->cb = pfunc;
    pt->arg = arg;

    return 0;
}

inline
int ftimer_start(ftimer* pt)
{
    if (timer_settime(pt->timerid, 0, &pt->its, NULL) == -1)
        return 1;
    return 0;
}

inline
int ftimer_del(ftimer* pt)
{
    return timer_delete(pt->timerid);
}

int ftimerfd_create()
{
    int fd = timerfd_create(CLOCKID, TFD_NONBLOCK);

    if( fd == -1 )
        return -1;
    return fd;
}

int ftimerfd_start(int fd, long long nsesc, long long alter)
{
    struct itimerspec new_value;
    new_value.it_value.tv_sec = (time_t)(nsesc / TRANS_STONS);
    new_value.it_value.tv_nsec = (long int)(nsesc % TRANS_STONS);
    new_value.it_interval.tv_sec = (time_t)(alter / TRANS_STONS);
    new_value.it_interval.tv_nsec = (long int)(alter % TRANS_STONS);

    if ( timerfd_settime(fd, 0, &new_value, NULL) == -1 ) {
        return 1;
    } else {
        return 0;
    }
}

int ftimerfd_stop(int fd)
{
    struct itimerspec new_value;
    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 0;

    if ( timerfd_settime(fd, 0, &new_value, NULL) == -1 )
        return 1;
    return 0;
}

unsigned long long ftime_gettime()
{
    struct timeval tv;
    int ret = gettimeofday(&tv, NULL);
    if (ret) {
        return 0;
    }

    return (unsigned long long)tv.tv_sec * 1000000l +
           (unsigned long long)tv.tv_usec;
}
