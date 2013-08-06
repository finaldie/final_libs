/*
 * =====================================================================================
 *
 *       Filename:  fev_timer_service.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/27/2013 04:23:56 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef _FEV_TIMER_SERVICE_H_
#define _FEV_TIMER_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "fev_timer.h"

typedef struct _fev_timer_svc fev_timer_svc;
typedef struct _ftimer_node   ftimer_node;
typedef void (*ftimer_cb)(fev_state*, void* arg);

fev_timer_svc* fev_create_timer_service(
                fev_state*,
                uint32_t interval /* unit ms */
                );

int fev_delete_timer_service(fev_timer_svc*);

ftimer_node*   fev_tmsvc_add_timer(
                fev_timer_svc*,
                uint32_t expire, /* unit ms */
                ftimer_cb,
                void* arg);

// call this when user want to cancel it
int fev_tmsvc_del_timer(fev_timer_svc*, ftimer_node*);

// reset a timer, the timer start time will be set to
// the current time, you also can use del_timer and add_timer
// to achieve that, but it will cost more memory and may have
// the performance impact
int fev_tmsvc_reset_timer(fev_timer_svc*, ftimer_node*);

#ifdef __cplusplus
}
#endif

#endif
