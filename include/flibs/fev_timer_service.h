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
                uint32_t precision,     // unit millisecond
                fev_tmsvc_model_t type
                );

void fev_tmsvc_destroy(fev_timer_svc*);

/**
 * Trigger all expired timers manually
 *
 * @return  How many valid timers expired and be triggered
 */
int fev_tmsvc_process(fev_timer_svc*);

/**
 * Get the first valid expired timer node
 */
ftimer_node* fev_tmsvc_top(fev_timer_svc*);

ftimer_node* fev_tmsvc_timer_add(
               fev_timer_svc*,
               long delay,       // Initial delay. unit millisecond
               long interval,    // Interval if > 0. unit millisecond
               ftimer_cb,
               void* arg);

/**
 * call this when user want to cancel it
 */
int fev_tmsvc_timer_del(ftimer_node*);

/**
 * Reset a timer, the timer start time will be set to
 * the current time, you also can use del_timer and add_timer
 * to achieve that, but it will cost more memory and may have
 * the performance impact.
 *
 * @note  It's no effect if the timer_node is invalid
 */
int   fev_tmsvc_timer_reset(ftimer_node*);

int   fev_tmsvc_timer_resetn(ftimer_node*, long expiration);

long  fev_tmsvc_timer_starttime(const ftimer_node*);

long  fev_tmsvc_timer_remaining(const ftimer_node*);

long  fev_tmsvc_timer_expiration(const ftimer_node*);

long  fev_tmsvc_timer_interval(const ftimer_node*);

int   fev_tmsvc_timer_valid(const ftimer_node*);

void* fev_tmsvc_timer_data(const ftimer_node*);

#ifdef __cplusplus
}
#endif

#endif

