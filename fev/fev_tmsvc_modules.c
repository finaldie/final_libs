#include "fev_tmsvc_modules.h"

int fev_tmmod_timeout(struct timespec* start, struct timespec* now, uint32_t expire)
{
    long int start_time_ns = start->tv_sec * NS_PER_SECOND + start->tv_nsec;
    long int now_time_ns = now->tv_sec * NS_PER_SECOND + now->tv_nsec;

    if( now_time_ns >= (start_time_ns + expire * NS_PER_MS) ) {
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
