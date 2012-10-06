/*
 * =====================================================================================
 *    Filename:  fco.h
 *    Description: C coroutine
 *    Version:  1.0
 *    Created:  09/30/2012 19:04:15
 *    Compiler:  gcc
 *    Author:  finaldie
 * =====================================================================================
 */

#ifndef _F_COROUTINE_H_
#define _F_COROUTINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FCO_STATUS_DEAD    0
#define FCO_STATUS_READY   1
#define FCO_STATUS_RUNNING 2
#define FCO_STATUS_SUSPEND 3

typedef struct _fco_sched fco_sched;
typedef struct _fco fco;

typedef void* (*pfunc_co)(fco*, void* arg);

fco_sched* fco_scheduler_create();
void       fco_scheduler_destroy(fco_sched*);

fco*       fco_main_create(fco_sched* sched, pfunc_co);
fco*       fco_create(fco*, pfunc_co);
void*      fco_resume(fco*, void*);
void*      fco_yield(fco*, void*);
int        fco_status(fco*);


#ifdef __cplusplus
}
#endif

#endif
