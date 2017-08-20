#ifndef FTIME_H
#define FTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/**
 * POSIX timer which be triggered by signal, only be avaliable when:
 *   _POSIX_C_SOURCE >= 199309L
 */

#if defined _POSIX_C_SOURCE && _POSIX_C_SOURCE >= 199309L

# include <signal.h>

typedef void (*ftimer_cb)(void*);
typedef struct _f_timer {
    timer_t timerid;
    struct  sigevent sev;
    struct  itimerspec its;

    ftimer_cb cb;
    void*   arg;
} ftimer;

int ftimer_create(ftimer*, long long nsecs, long long interval,
                 ftimer_cb pfunc, void* arg);
int ftimer_start(ftimer*);
int ftimer_del(ftimer*);

#endif   // _POSIX_C_SOURCE macro detection

/**
 * Linux specific timer which be triggered by a file descriptor
 */

#if defined __linux__

int ftimerfd_create();
int ftimerfd_start(int fd, long long nsecs, long long interval);
int ftimerfd_stop(int fd);

#endif  // __linux__

/**
 * Return A `long long` time value, unit: micro-seconds. it's equal to:
 *  current time second * 1000000 + current usec
 */
unsigned long long ftime_gettime();

#ifdef __cplusplus
}
#endif

#endif // FTIME_H

