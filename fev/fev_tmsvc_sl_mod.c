#ifndef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "flibs/fdlist.h"
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
    tm_sl_data* sl_data = calloc(1, sizeof(tm_sl_data));
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

        if (node && node->isvalid && node->cb) {
            if (fev_tmmod_timeout(&node->start, now, node->expiration)) {
                node->cb(fev, node->arg);
                _destroy_timer(timer_node);
                expired++;
            } else {
                fdlist_push(sl_data->backup_list, timer_node);
            }
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

typedef struct first_expired {
    ftimer_node* node;
} first_expired;

static
int _node_each(fdlist_node_t* node, void* ud) {
    ftimer_node* timer_node = fdlist_get_nodedata(node);
    if (!timer_node->isvalid) {
        return 0;
    }

    first_expired* first = ud;
    if (first->node) {
        long timer_ms = TIMEMS(timer_node->start)  + timer_node->expiration;
        long min_ms   = TIMEMS(first->node->start) + first->node->expiration;

        if (timer_ms < min_ms) {
            first->node = timer_node;
        }
    } else {
        first->node = timer_node;
    }
    return 0;
}

static
ftimer_node* _first(void* mod_data) {
    tm_sl_data* sl_data = mod_data;
    first_expired first = {NULL};

    fdlist_foreach(sl_data->timer_list, _node_each, &first);
    return first.node;
}

fev_tmsvc_opt sl_opt = {
    .init    = _init,
    .destroy = _destroy,
    .process = _process,
    .add     = _add,
    .del     = _del,
    .first   = _first
};
