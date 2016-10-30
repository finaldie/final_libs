#ifndef FEV_TIMER_H
#define FEV_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <flibs/fev.h>

typedef struct fev_timer fev_timer;
typedef void (*fev_timer_cb)(fev_state*, void* arg);

fev_timer* fev_add_timer_event(
        fev_state* fev,
        long long first,  /* unit nano second */
        long long alter, /* unit nano second */
        fev_timer_cb,
        void* arg);

int     fev_del_timer_event(fev_state* fev, fev_timer*);

#ifdef __cplusplus
}
#endif

#endif

