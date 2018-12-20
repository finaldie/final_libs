#ifndef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "flibs/compiler.h"
#include "flibs/fdlist.h"
#include "flibs/fev_timer_service.h"
#include "fev_tmsvc_modules.h"

typedef struct tm_single_linked_data {
    fdlist* timer_list;
    fdlist* backup_list;
} tm_sl_data;

static
int _destroy_timer(fdlist_node_t* timer_node)
{
    ftimer_node* node = fdlist_get_nodedata(timer_node);
    free(node);
    fdlist_destroy_node(timer_node);
    return 0;
}

static
int _destroy_timerlist(fdlist* timer_list)
{
    fdlist_node_t* timer_node = NULL;
    while ((timer_node = fdlist_pop(timer_list))) {
        _destroy_timer(timer_node);
    }

    fdlist_destroy(timer_list);
    return 0;
}

static
int _init(void** mod_data)
{
    tm_sl_data* sl_data  = calloc(1, sizeof(tm_sl_data));
    sl_data->timer_list  = fdlist_create();
    sl_data->backup_list = fdlist_create();

    *mod_data = sl_data;
    return 0;
}

static
void _destroy(void* mod_data)
{
    if (!mod_data) {
        return;
    }

    tm_sl_data* sl_data = mod_data;
    _destroy_timerlist(sl_data->timer_list);
    _destroy_timerlist(sl_data->backup_list);
    free(sl_data);
}

static
int _process(fev_state* fev, void* mod_data, struct timespec* now)
{
    tm_sl_data* sl_data = mod_data;
    fdlist_node_t* timer_node = NULL;
    int expired = 0;

    while ((timer_node = fdlist_pop(sl_data->timer_list))) {
        ftimer_node* node = fdlist_get_nodedata(timer_node);

        if (unlikely(!node || !node->valid || !node->cb)) {
            _destroy_timer(timer_node);
            continue;
        }

        int timeout = fev_tmmod_timeout(node, now);
        if (timeout) {
            node->cb(fev, node->arg);

            // Re-calculate expiration time
            if (node->interval > 0) {
                node->expiration = node->interval;
                node->start.tv_sec  += node->expiration / MS_PER_SECOND;
                node->start.tv_nsec += (node->expiration * NS_PER_MS) % NS_PER_SECOND;
            }

            node->triggered = 1;
            expired++;
        }

        if (node->interval > 0 || !timeout) {
            fdlist_push(sl_data->backup_list, timer_node);
        } else {
            _destroy_timer(timer_node);
        }
    }

    // util now, the timer_list is empty, then
    // swap the timer_list and backup_list, since everytime we
    // iterate the timer_node from timer_list
    fdlist* tmp = sl_data->timer_list;
    sl_data->timer_list = sl_data->backup_list;
    sl_data->backup_list = tmp;

    return expired;
}

static
int _add(ftimer_node* node, void* mod_data)
{
    tm_sl_data* sl_data = mod_data;

    // since we don't care about the storage of the list, so just put
    // the second 'size' parameter as '0'
    fdlist_node_t* timer_node = fdlist_make_node(node, sizeof(ftimer_node));
    fdlist_push(sl_data->timer_list, timer_node);
    return 0;
}

static
int _del(ftimer_node* node __attribute__((unused)),
         void* mod_data    __attribute__((unused)))
{
    return 0;
}

typedef struct top_expired {
    ftimer_node* node;
} top_expired;

static
int _node_each(fdlist_node_t* node, void* ud) {
    ftimer_node* timer_node = fdlist_get_nodedata(node);
    if (!timer_node->valid) {
        return 0;
    }

    top_expired* top = ud;
    if (top->node) {
        long timer_ms = TIMEMS(timer_node->start)  + timer_node->expiration;
        long min_ms   = TIMEMS(top->node->start) + top->node->expiration;

        if (timer_ms < min_ms) {
            top->node = timer_node;
        }
    } else {
        top->node = timer_node;
    }
    return 0;
}

static
ftimer_node* _top(void* mod_data) {
    tm_sl_data* sl_data = mod_data;
    top_expired top = {NULL};

    fdlist_foreach(sl_data->timer_list, _node_each, &top);
    return top.node;
}

fev_tmsvc_ops_t sl_ops = {
    .init    = _init,
    .destroy = _destroy,
    .process = _process,
    .add     = _add,
    .del     = _del,
    .top     = _top
};
