/*
 * =============================================================================
 *
 *       Filename:  fev_timer_service.c
 *
 *    Description:  The framework of Timer Serivce.
 *                  Timer Service compose of user api, service framwork, module
 *                  sdk and modules.
 *
 *                  user api: fev_timer_service.h fev_tmsvc_types.h
 *                            support the add/del/reset api
 *
 *                  service framework: fev_timer_service.c
 *                            service framework is responsible for:
 *                              1. construct ftimer_node by the user input
 *                              2. communicate with fev state machie to setup
 *                                 the cron timer for checking the ftimer_node
 *                                 status every 1ms
 *                              3. drive the module to do the
 *                                 init/destroy/add/del/reset job
 *
 *                  module sdk: fev_tmsvc_modules.h
 *                            support the api for module developers
 *
 *                  modules: fev_tmsvc_sl_mod.c
 *                            the simple version of the single linked timer
 *                            system, responsible for:
 *                              1. storage system for ftimer_nodes
 *                              2. add/del/reset implementation
 *                              3. in the every loop of checking, remove the
 *                                 inactive timer nodes, and run the callback
 *                                 functions for those timeout timer nodes'
 *
 * =============================================================================
 */

#ifndef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "flibs/compiler.h"
#include "fev_tmsvc_modules.h"
#include "flibs/fev_timer_service.h"

#ifndef CLOCK_MONOTONIC_COARSE
#define CLOCK_MONOTONIC_COARSE 6
#endif

struct _fev_timer_svc {
    fev_state*       fev;
    fev_timer*       ftimer;
    void*            mod_data;
    fev_tmsvc_ops_t* ops;
    uint32_t         interval; // ms
    clockid_t        clockid;
};

static
int _process_expired(fev_timer_svc* svc) {
    // Get current timestamp
    struct timespec now;
    if (unlikely(clock_gettime(svc->clockid, &now))) {
        fprintf(stderr, "FATAL ERROR! timer service cannot get current "
                "timestamp:%s\n", strerror(errno));
        abort();
    }

    // Process the expired timers
    return svc->ops->process(svc->fev, svc->mod_data, &now);
}

static
void timer_svc_callback(fev_state* fev, void* arg) {
    _process_expired(arg);
}

fev_timer_svc* fev_tmsvc_create(
                fev_state* fev,
                uint32_t interval,      // unit millisecond
                fev_tmsvc_model_t type) {
    if (!fev) {
        return NULL;
    }

    fev_timer_svc* timer_svc = calloc(1, sizeof(fev_timer_svc));

    timer_svc->fev = fev;
    timer_svc->interval = interval;

    if (interval) {
        long long te_interval = (long long)interval * NS_PER_MS;
        timer_svc->ftimer = fev_add_timer_event(fev, te_interval, te_interval,
                                                timer_svc_callback, timer_svc);
    }

    // init the opt table
    timer_svc->ops = tmsvc_ops_tbl[type];
    int mod_init_ret = timer_svc->ops->init(&timer_svc->mod_data);

    if (mod_init_ret) {
        timer_svc->ops->destroy(timer_svc->mod_data);
        fev_del_timer_event(fev, timer_svc->ftimer);
        free(timer_svc);
        return NULL;
    }

    timer_svc->clockid = CLOCK_MONOTONIC;

    struct timespec resolution;
    if (0 == clock_getres(CLOCK_MONOTONIC_COARSE, &resolution)) {
        if (resolution.tv_nsec <= NS_PER_MS) {
            timer_svc->clockid = CLOCK_MONOTONIC_COARSE;
        }
    }

    return timer_svc;
}

void fev_tmsvc_destroy(fev_timer_svc* svc) {
    if (!svc) {
        return;
    }

    // must delete the timer event at first, then we
    // can delete all the other structures safety
    fev_del_timer_event(svc->fev, svc->ftimer);
    svc->ops->destroy(svc->mod_data);
    free(svc);

    return;
}

int fev_tmsvc_process(fev_timer_svc* svc) {
    return svc ? _process_expired(svc) : -1;
}

ftimer_node* fev_tmsvc_top(fev_timer_svc* svc) {
    return svc ? svc->ops->top(svc->mod_data) : NULL;
}

ftimer_node* fev_tmsvc_timer_add(
                fev_timer_svc* svc,
                long delay, /* unit ms */
                long interval, /* unit ms */
                ftimer_cb timer_cb,
                void* arg) {
    if (unlikely(!svc || delay < 0 || interval < 0 || !timer_cb)) {
        return NULL;
    }

    ftimer_node* node = calloc(1, sizeof(ftimer_node));

    if (unlikely(clock_gettime(svc->clockid, &node->start))) {
        free(node);
        return NULL;
    }

    node->cb         = timer_cb;
    node->arg        = arg;
    node->owner      = svc;
    node->interval   = interval;
    node->expiration = delay;
    node->valid      = 1;

    // call the module add method to do the actual adding action
    if (unlikely(svc->ops->add(node, svc->mod_data))) {
        free(node);
        return NULL;
    }

    return node;
}

int  fev_tmsvc_timer_del(ftimer_node* node) {
    if (unlikely(!node || !node->owner)) {
        return 1;
    }

    // only set the node as invalid,
    node->valid = 0;

    // run private del method
    fev_timer_svc* svc = node->owner;
    if (svc->ops->del) {
        return svc->ops->del(node, svc->mod_data);
    }

    return 0;
}

int fev_tmsvc_timer_reset(ftimer_node* node) {
    if (unlikely(!node || !node->owner)) {
        return 1;
    }

    fev_timer_svc* svc = node->owner;
    if (unlikely(clock_gettime(svc->clockid, &node->start))) {
        return 1;
    }

    return 0;
}

int fev_tmsvc_timer_resetn(ftimer_node* node, long expiration) {
    if (unlikely(!node || !node->owner || expiration < 0)) {
        return 1;
    }

    fev_timer_svc* svc = node->owner;
    if (unlikely(clock_gettime(svc->clockid, &node->start))) {
        return 1;
    }

    node->expiration = expiration;
    return 0;
}

int fev_tmsvc_timer_valid(const ftimer_node* node) {
    return node && node->valid;
}

long  fev_tmsvc_timer_starttime(const ftimer_node* node) {
    return node ? TIMEMS(node->start) : -1;
}

long fev_tmsvc_timer_expiration(const ftimer_node* node) {
    if (unlikely(!node)) return LONG_MIN;
    fev_timer_svc* svc = node->owner;

    // Get current timestamp
    struct timespec now;
    if (unlikely(clock_gettime(svc->clockid, &now))) {
        fprintf(stderr, "FATAL ERROR! timer service cannot get current "
                "timestamp:%s\n", strerror(errno));
        abort();
    }

    return TIMEMS(node->start) + node->expiration - TIMEMS(now);
}

void* fev_tmsvc_timer_data(const ftimer_node* node) {
    return node ? node->arg : NULL;
}
