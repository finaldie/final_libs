#include <stdio.h>
#include "fev_tmsvc_modules.h"

// The resolution is million seconds for this API
int fev_tmmod_timeout(struct timespec* start, struct timespec* now,
                      long expiration)
{
    long int diff_sec = now->tv_sec - start->tv_sec;
    long int diff_nsec = now->tv_nsec - start->tv_nsec;

    // if the timer will be started in the future, exit with no-timeout
    if (diff_sec < 0) {
        return 0;
    }

    if (diff_sec == 0 && diff_nsec < 0) {
        return 0;
    }

    long int diff_ms = diff_sec * MS_PER_SECOND + (diff_nsec / NS_PER_MS);

    if (diff_ms >= expiration) {
        return 1;
    } else {
        return 0;
    }
}

extern fev_tmsvc_opt sl_opt;

// global opt table
// every type of timer module should register in this table
fev_tmsvc_opt* tmsvc_opt_tbl[] = {
    &sl_opt,   // for FEV_TMSVC_SINGLE_LINKED
    NULL       // reserve for FEV_TMSVC_TIMER_WHEEL
};
