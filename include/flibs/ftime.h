#ifndef FTIME_H
#define FTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

typedef void (*ftimer_cb)(void*);
typedef struct _f_timer {
    timer_t timerid;
    struct  sigevent sev;
    struct  itimerspec its;

    ftimer_cb cb;
    void*   arg;
} ftimer;

int    ftimer_create(ftimer*, long long nsecs, long long alter,
                    ftimer_cb pfunc, void* arg);
int    ftimer_start(ftimer*);
int    ftimer_del(ftimer*);

#ifdef __linux__
# ifndef HAVE_TIMERFD_H
# define HAVE_TIMERFD_H 1

# include <sys/timerfd.h>

int    ftimerfd_create();
int    ftimerfd_start(int fd, long long nsecs, long long alter);
int    ftimerfd_stop(int fd);
# endif
#endif

// return a time value its format is:
// current time * 1000000 + current usec
unsigned long long fgettime();

#ifdef __cplusplus
}
#endif

#endif

