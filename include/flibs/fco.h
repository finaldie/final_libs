#ifndef FCOROUTINE_H
#define FCOROUTINE_H

/**
 * fco context, to help user make huge amount of user contexts, also provide
 *  the basic interfaces like yield/resume to build the application easier.
 *
 * Notes: This fco_context is not recommended to use, since it may leading
 *  portability issues. And in certain 64bit platform may be not working
 *  as expected.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define FCO_STATUS_DEAD    0
#define FCO_STATUS_READY   1
#define FCO_STATUS_RUNNING 2
#define FCO_STATUS_SUSPEND 3

#define FCO_TYPE_ALONE 0
#define FCO_TYPE_CHILD 1

typedef struct _fco_sched fco_sched;
typedef struct _fco fco;

typedef void* (*pfunc_co)(fco*, void* arg);
typedef void (*phook_cb)(fco*, void* arg);
typedef void (*plugin_init)(fco_sched*, void* arg);

fco_sched* fco_scheduler_create();
void       fco_scheduler_destroy(fco_sched*);

fco*       fco_main_create(fco_sched*, pfunc_co);
fco*       fco_create(fco*, pfunc_co, int type);
void*      fco_resume(fco*, void*);
void*      fco_yield(fco*, void*);
int        fco_status(fco*);
void       fco_register_plugin(fco_sched*, void* arg, plugin_init init,
                               phook_cb before_sw, phook_cb after_sw);

#ifdef __cplusplus
}
#endif

#endif

