#include <stdlib.h>
#include <string.h>

#include "flist/fdlist.h"
#include "fev_tmsvc_modules.h"

typedef struct tm_single_linked_data {
    fdlist* timer_list;
    fdlist* backup_list;
} tm_sl_data;

static
int _destroy_timer(fdlist_node_t* timer_node)
{
    ftimer_node* node = (ftimer_node*)fdlist_get_nodedata(timer_node);
    free(node);
    fdlist_destroy_node(timer_node);
    return 0;
}

static
int _destroy_timerlist(fdlist* timer_list)
{
    fdlist_node_t* timer_node = NULL;
    while( (timer_node = fdlist_pop(timer_list) ) ) {
        _destroy_timer(timer_node);
    }

    fdlist_destroy(timer_list);
    return 0;
}

static
int fev_tmmod_single_linked_init(void** mod_data)
{
    tm_sl_data* sl_data = malloc(sizeof(tm_sl_data));
    memset(sl_data, 0, sizeof(tm_sl_data));

    sl_data->timer_list = fdlist_create();
    sl_data->backup_list = fdlist_create();

    *mod_data = sl_data;
    return 0;
}

static
void fev_tmmod_single_linked_destroy(void* mod_data)
{
    if (!mod_data) {
        return;
    }

    tm_sl_data* sl_data = (tm_sl_data*)mod_data;
    _destroy_timerlist(sl_data->timer_list);
    _destroy_timerlist(sl_data->backup_list);
    free(sl_data);
}

static
void fev_tmmod_single_linked_loopcb(fev_state* fev, void* mod_data, struct timespec* now)
{
    tm_sl_data* sl_data = (tm_sl_data*)mod_data;

    fdlist_node_t* timer_node = NULL;
    while( (timer_node = fdlist_pop(sl_data->timer_list) ) ) {
        ftimer_node* node = (ftimer_node*)fdlist_get_nodedata(timer_node);
        if( node && node->isvalid && node->cb ) {
            if( fev_tmmod_timeout(&node->start, now, node->expire) ) {
                node->cb(fev, node->arg);
                _destroy_timer(timer_node);
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
}

static
int fev_tmmod_single_linked_add(ftimer_node* node, void* mod_data)
{
    tm_sl_data* sl_data = (tm_sl_data*)mod_data;

    // since we don't care about the storage of the list, so just put
    // the second 'size' parameter as '0'
    fdlist_node_t* timer_node = fdlist_make_node(node, sizeof(ftimer_node));
    fdlist_push(sl_data->timer_list, timer_node);
    return 0;
}

static
int fev_tmmod_single_linked_del(ftimer_node* node, void* mod_data)
{
    return 0;
}

static
int fev_tmmod_single_linked_reset(ftimer_node* node, void* mod_data)
{
    return 0;
}

fev_tmsvc_opt sl_opt = {
    fev_tmmod_single_linked_init,
    fev_tmmod_single_linked_destroy,
    fev_tmmod_single_linked_loopcb,
    fev_tmmod_single_linked_add,
    fev_tmmod_single_linked_del,
    fev_tmmod_single_linked_reset
};
