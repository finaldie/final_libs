/**
 *  This header file expose to user, to declare some public types
 */


#ifndef _FEV_TIMER_SERVICE_TYPES_H_
#define _FEV_TIMER_SERVICE_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "fev.h"

#define NS_PER_SECOND 1000000000L
#define NS_PER_MS     1000000L

#define MS_PER_SECOND 1000

typedef enum fev_tmsvc_model_t {
    FEV_TMSVC_SINGLE_LINKED = 0,
    FEV_TMSVC_TIMER_WHEEL = 1     // NOT SUPPORT
} fev_tmsvc_model_t;

typedef struct _ftimer_node   ftimer_node;
typedef struct _fev_timer_svc fev_timer_svc;

typedef void (*ftimer_cb)(fev_state*, void* arg);

#ifdef __cplusplus
}
#endif

#endif
