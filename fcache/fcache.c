#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "fhash.h"
#include "fcache_list.h"
#include "fcache.h"

#define FCACHE_BALANCE_INTERVAL 2000

struct _fcache {
    f_hash*  phash_node_index;
    fc_list* pactive_list;
    fc_list* pinactive_list;
    cache_obj_free obj_free;
    size_t   max_size;
    size_t   op_count;
    int      enable_balance;
};

fcache_t* fcache_create(size_t max_size, cache_obj_free obj_free)
{
    if ( !max_size ) return NULL;

    fcache_t* pcache = malloc(sizeof(fcache_t));
    if ( !pcache ) return NULL;
    memset(pcache, 0, sizeof(fcache_t));

    pcache->phash_node_index = hash_create(max_size * 1.5);
    if ( !pcache->phash_node_index ) {
        fcache_destroy(pcache);
        return NULL;
    }

    pcache->pactive_list = fcache_list_create();
    if ( !pcache->pactive_list ) {
        fcache_destroy(pcache);
        return NULL;
    }

    pcache->pinactive_list = fcache_list_create();
    if ( !pcache->pinactive_list ) {
        fcache_destroy(pcache);
        return NULL;
    }

    pcache->obj_free = obj_free;
    pcache->max_size = max_size;
    pcache->op_count = 0;
    pcache->enable_balance = 0;

    return pcache;
}

static inline
void      _fcache_destroy_list(fc_list* plist, cache_obj_free obj_free)
{
    fcache_node_t* node = NULL;
    while ( (node = fcache_list_pop(plist)) ) {
        if ( obj_free ) {
            obj_free(fcache_list_get_nodedata(node));
        }
        fcache_list_destroy_node(node);
    }

    fcache_list_destroy(plist);
}

void      fcache_destroy(fcache_t* pcache)
{
    if ( !pcache ) return;
    hash_delete(pcache->phash_node_index);
    _fcache_destroy_list(pcache->pactive_list, pcache->obj_free);
    _fcache_destroy_list(pcache->pinactive_list, pcache->obj_free);
    free(pcache);
}

static inline
int       _fcache_add_node(fcache_t* pcache, const char* key, size_t key_size,
                           void* value, size_t value_size)
{
    fcache_node_t* add_node = fcache_list_make_node(key_size);
    if ( !add_node ) return 1;
    fcache_list_set_nodekey(add_node, key, key_size);
    fcache_list_set_nodedata(add_node, value);

    // for a new node
    // 1. add into hash table
    // 2. add into inactive node
    hash_set_str(pcache->phash_node_index, key, add_node);
    if ( fcache_list_push(pcache->pinactive_list, add_node, value_size) ) {
        hash_del_str(pcache->phash_node_index, key);
        fcache_list_destroy_node(add_node);
    }

    return 0;
}

static inline
int       _fcache_update_node(fcache_t* pcache, fcache_node_t* node, void* value,
                              size_t value_size)
{
    // call obj_free only when the target node has changed
    void* user_data = fcache_list_get_nodedata(node);
    if ( user_data != value && pcache->obj_free ) {
        pcache->obj_free(user_data);
    }

    fcache_list_set_nodedata(node, value);
    fcache_list_update_node(node, value_size);

    return 0;
}

static inline
void      _fcache_del_node(fcache_t* pcache, fcache_node_t* node)
{
    const char* key = fcache_list_get_nodekey(node);
    if ( pcache->obj_free ) {
        pcache->obj_free(fcache_list_get_nodedata(node));
    } else {
        fprintf(stderr, "[WARNING]: data delete may cause memory leak, key = %s\n", key);
    }

    hash_del_str(pcache->phash_node_index, key);
    fcache_list_delete_node(node);
    fcache_list_destroy_node(node);
}

static inline
int       _fcache_move_node(fcache_t* pcache, fcache_node_t* node)
{
    if ( !fcache_list_move_node(node, pcache->pactive_list) ) {
        return 0;
    } else {
        return 1;
    }
}

static inline
int       _fcache_balance_nodes(fcache_t* pcache)
{
    size_t inactive_nodes_count = fcache_list_size(pcache->pinactive_list);
    size_t active_nodes_count = fcache_list_size(pcache->pactive_list);
    size_t avg_length = ( inactive_nodes_count + active_nodes_count ) / 2;
    if ( active_nodes_count <= avg_length ) {
        return 0;
    }

    size_t need_move_count = active_nodes_count - avg_length;
    size_t i = 0;
    for ( ; i < need_move_count; i++ ) {
        fcache_node_t* node = fcache_list_pop(pcache->pactive_list);
        if ( !node ) break;
        fcache_list_push(pcache->pinactive_list, node, fcache_list_node_size(node));
    }

    return 0;
}

static inline
size_t    _fcache_free_size(fcache_t* pcache)
{
    return pcache->max_size - fcache_list_data_size(pcache->pactive_list) -
            fcache_list_data_size(pcache->pinactive_list);
}

/**
 * brief check and drop nodes from inactive list
 */
static inline
int       _fcache_check_and_drop_nodes(fcache_t* pcache, fcache_node_t* node,
                                  size_t target_size)
{
    size_t grow_size = 0;
    size_t inactive_size = fcache_list_data_size(pcache->pinactive_list);
    size_t free_size = _fcache_free_size(pcache);

    if ( !node ) { // add new node
        grow_size = target_size;
    } else { // update node
        size_t node_data_size = fcache_list_node_size(node);
        grow_size = node_data_size <= target_size ? target_size - node_data_size : 0;
    }

    // have enough size for growing
    if ( grow_size == 0 || free_size >= grow_size ) {
        return 0;
    }

    // there no enough free space, need to drop some nodes
    // first calculatie the size we need to drop
    size_t drop_size = grow_size - free_size;

    // second check inactive list whether have enough space to drop
    // if no, balance once and try again
    if ( inactive_size < drop_size ) {
        _fcache_balance_nodes(pcache);
        inactive_size = fcache_list_data_size(pcache->pinactive_list);
        if ( inactive_size < drop_size ) {
            return 1;
        }
    }

    // when this is a updating mode, move node to inactive list tail
    if ( node ) {
        fcache_list_move_node(node, pcache->pinactive_list);
    }

    // inactive list has enough space to drop
    size_t dropped_size = 0;
    while ( dropped_size < drop_size ) {
        fcache_node_t* dropped_node = fcache_list_pop(pcache->pinactive_list);
        if ( !dropped_node ) break;
        dropped_size += fcache_list_node_size(dropped_node);
        _fcache_del_node(pcache, dropped_node);
    }

    return 0;
}

/**
 * brief @ first incoming, we push obj into inactive list
 *       sencond incoming(fcache_get_obj trigger it), we move obj into active
 *       list if the obj has not dropped from inactive list
 *       @ if the node count will reach the max limitation, start the balance
 *       mechanism
 *       @ remove node only from inactive list
 */
int       fcache_set_obj(fcache_t* pcache, const char* key, size_t key_size,
                         void* value, size_t value_size)
{
    if ( !pcache || !key || !key_size ) return 1;
    if ( value && !value_size ) return 1;
    if ( value_size > pcache->max_size ) return 1;

    fcache_node_t* node = hash_get_str(pcache->phash_node_index, key);
    if ( _fcache_check_and_drop_nodes(pcache, node, value_size) ) {
        return 1;
    }

    int ret = 0;
    if ( likely(!node) ) {
        ret = _fcache_add_node(pcache, key, key_size, value, value_size);
    } else {
        if ( value ) {
            ret = _fcache_update_node(pcache, node, value, value_size);
        } else {
            _fcache_del_node(pcache, node);
        }
    }

    // check & enable balance
    if ( unlikely(!pcache->enable_balance && !ret) ) {
        size_t current_size = fcache_list_data_size(pcache->pinactive_list) +
                              fcache_list_data_size(pcache->pactive_list);
        size_t inactive_nodes_count = fcache_list_size(pcache->pinactive_list);
        size_t active_nodes_count = fcache_list_size(pcache->pactive_list);
        size_t avg_node_size = current_size /
                               (inactive_nodes_count + active_nodes_count);
        size_t times = (pcache->max_size - current_size) / avg_node_size;

        if ( times <= FCACHE_BALANCE_INTERVAL )
            pcache->enable_balance = 1;
    }

    return ret;
}

void*     fcache_get_obj(fcache_t* pcache, const char* key)
{
    if ( !pcache || !key ) return NULL;

    if ( likely(pcache->enable_balance) ) {
        pcache->op_count++;

        if ( pcache->op_count == FCACHE_BALANCE_INTERVAL ) {
            pcache->op_count = 0;
            _fcache_balance_nodes(pcache);
        }
    }

    fcache_node_t* node = hash_get_str(pcache->phash_node_index, key);
    if ( unlikely(!node) )
        return NULL;

    // if in acitve list
    if ( fcache_list_node_owner(node) == pcache->pactive_list ) {
        // move it to head of the list when size of list > 1
        fcache_list_move_node(node, pcache->pactive_list);
        return fcache_list_get_nodedata(node);
    } else {
        // we don't care the result of moving, if failure, try it next time
        _fcache_move_node(pcache, node);
        return fcache_list_get_nodedata(node);
    }
}

