/*
 * =====================================================================================
 *
 *       Filename:  fev_timer.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/18/2011 16:58:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef _FEV_TIMER_H_
#define _FEV_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "fev.h"

typedef struct fev_timer fev_timer;
typedef void (*fev_timer_cb)(void* arg);

fev_timer* fev_add_timer_event(
        fev_state* fev, 
        long long nsec, 
        long long alter, 
        fev_timer_cb, 
        void* arg);

int     fev_del_timer_event(fev_state* fev, fev_timer*);

#ifdef __cplusplus
}
#endif

#endif
