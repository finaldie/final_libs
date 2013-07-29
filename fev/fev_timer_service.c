/*
 * =====================================================================================
 *
 *       Filename:  fev_timer_service.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/27/2013 05:20:10 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "flist/fdlist.h"
#include "fev_timer_service.h"

#define NS_PER_SECOND 1000000000L
#define NS_PER_MS     1000000L

struct _ftimer_node {
    struct timespec start;
    ftimer_cb       cb;
    void*           arg;
    fdlist_node_t*  owner;
    uint32_t        expire;
    int             isvalid;
};

struct _fev_timer_svc {
    fev_state* fev;
    fev_timer* ftimer;
    fdlist*    timer_list;
    fdlist*    backup_list;
    uint32_t   interval;
};

static
int is_trigger_timer(struct timespec* start, struct timespec* now, uint32_t expire)
{
    long int start_time_ns = start->tv_sec * NS_PER_SECOND + start->tv_nsec;
    long int now_time_ns = now->tv_sec * NS_PER_SECOND + now->tv_nsec;

    if( (start_time_ns + expire * NS_PER_MS) <= now_time_ns ) {
        return 1;
    } else {
        return 0;
    }
}

static
int fev_tmsvc_destroy_timer(fev_timer_svc* svc, fdlist_node_t* timer_node)
{
    ftimer_node* node = (ftimer_node*)fdlist_get_nodedata(timer_node);
    free(node);
    fdlist_destroy_node(timer_node);
    return 0;
}

static
void timer_svc_callback(fev_state* fev, void* arg)
{
    fev_timer_svc* svc = (fev_timer_svc*)arg;

    // get current timestamp
    struct timespec now;
    if( clock_gettime(CLOCK_MONOTONIC_COARSE, &now) ) {
        fprintf(stderr, "FATAL ERROR! timer service cannot get current timestamp:%s\n", strerror(errno));
        abort();
    }

    fdlist_node_t* timer_node = NULL;
    while( (timer_node = fdlist_pop(svc->timer_list) ) ) {
        ftimer_node* node = (ftimer_node*)fdlist_get_nodedata(timer_node);
        if( node && node->isvalid && node->cb ) {
            if( is_trigger_timer(&node->start, &now, node->expire) ) {
                node->cb(fev, node->arg);
                fev_tmsvc_destroy_timer(svc, timer_node);
            } else {
                fdlist_push(svc->backup_list, timer_node);
            }
        } else {
            fev_tmsvc_destroy_timer(svc, timer_node);
        }
    }

    // util now, the timer_list is empty, then
    // swap the timer_list and backup_list, since everytime we
    // iterate the timer_node from timer_list
    fdlist* tmp = svc->timer_list;
    svc->timer_list = svc->backup_list;
    svc->backup_list = tmp;
}

fev_timer_svc* fev_create_timer_service(
                fev_state* fev,
                uint32_t interval /* unit ms */
                )
{
    if( !fev || !interval ) {
        return NULL;
    }

    fev_timer_svc* timer_svc = malloc(sizeof(fev_timer_svc));
    memset(timer_svc, 0, sizeof(fev_timer_svc));

    timer_svc->fev = fev;
    interval *= NS_PER_MS;
    timer_svc->ftimer = fev_add_timer_event(fev, interval,
                                            interval,
                                            timer_svc_callback,
                                            timer_svc);
    timer_svc->timer_list = fdlist_create();
    timer_svc->backup_list = fdlist_create();
    timer_svc->interval = interval;

    if( !timer_svc->ftimer || !timer_svc->timer_list ||
        !timer_svc->backup_list ) {
        fev_del_timer_event(fev, timer_svc->ftimer);
        fdlist_destroy(timer_svc->timer_list);
        fdlist_destroy(timer_svc->backup_list);
        free(timer_svc);
        return NULL;
    }

    return timer_svc;
}

int fev_delete_timer_service(fev_timer_svc* svc)
{
    if( !svc ) {
        return 1;
    }

    // must delete the timer event at first, then we
    // can delete all the other structures safety
    fev_del_timer_event(svc->fev, svc->ftimer);
    fdlist_destroy(svc->timer_list);
    fdlist_destroy(svc->backup_list);
    free(svc);

    return 0;
}

ftimer_node* fev_tmsvc_add_timer(
                fev_timer_svc* svc,
                uint32_t expire, /* unit ms */
                ftimer_cb timer_cb,
                void* arg)
{
    if( !svc || !expire || !timer_cb ) {
        return NULL;
    }

    ftimer_node* node = malloc(sizeof(ftimer_node));
    memset(node, 0, sizeof(ftimer_node));

    if( clock_gettime(CLOCK_MONOTONIC_COARSE, &node->start) ) {
        free(node);
        return NULL;
    }

    node->cb = timer_cb;
    node->arg = arg;
    node->owner = fdlist_make_node(node, sizeof(ftimer_node));
    node->expire = expire;
    node->isvalid = 1;
    if( !node->owner ) {
        free(node);
        return NULL;
    }

    fdlist_push(svc->timer_list, node->owner);

    return node;
}

int  fev_tmsvc_del_timer(fev_timer_svc* svc, ftimer_node* node)
{
    if( !svc || !node ) {
        return 1;
    }

    // only set the owner node as invalid,
    node->isvalid = 0;

    return 0;
}
