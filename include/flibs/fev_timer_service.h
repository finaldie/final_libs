#ifndef FEV_TIMER_SERVICE_H
#define FEV_TIMER_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <flibs/fev_timer.h>
#include <flibs/fev_tmsvc_types.h>

fev_timer_svc* fev_tmsvc_create(
                fev_state*,
                uint32_t interval,     // unit millisecond
                fev_tmsvc_model_t type
                );

void fev_tmsvc_destroy(fev_timer_svc*);

ftimer_node*   fev_tmsvc_timer_add(
                fev_timer_svc*,
                uint32_t expiration,       // unit millisecond
                ftimer_cb,
                void* arg);

// call this when user want to cancel it
int fev_tmsvc_timer_del(ftimer_node*);

// reset a timer, the timer start time will be set to
// the current time, you also can use del_timer and add_timer
// to achieve that, but it will cost more memory and may have
// the performance impact.
// Notes: It's no effect if the timer_node is invalid
int fev_tmsvc_timer_reset(ftimer_node*);

int fev_tmsvc_timer_resetn(ftimer_node*, uint32_t expiration);

#ifdef __cplusplus
}
#endif

#endif

