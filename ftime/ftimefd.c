#ifndef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <time.h>

#include "flibs/ftime.h"

#define TRANS_STONS (1000000000l)

#if defined __linux__

int ftimerfd_create()
{
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);

    if (fd == -1) return -1;
    return fd;
}

int ftimerfd_start(int fd, long long nsesc, long long alter)
{
    struct itimerspec new_value;
    new_value.it_value.tv_sec     = (time_t)  (nsesc / TRANS_STONS);
    new_value.it_value.tv_nsec    = (long int)(nsesc % TRANS_STONS);
    new_value.it_interval.tv_sec  = (time_t)  (alter / TRANS_STONS);
    new_value.it_interval.tv_nsec = (long int)(alter % TRANS_STONS);

    if (timerfd_settime(fd, 0, &new_value, NULL) == -1) {
        return 1;
    } else {
        return 0;
    }
}

int ftimerfd_stop(int fd)
{
    struct itimerspec new_value;
    new_value.it_value.tv_sec     = 0;
    new_value.it_value.tv_nsec    = 0;
    new_value.it_interval.tv_sec  = 0;
    new_value.it_interval.tv_nsec = 0;

    if ( timerfd_settime(fd, 0, &new_value, NULL) == -1 )
        return 1;
    return 0;
}

#endif

