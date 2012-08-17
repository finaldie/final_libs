#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "lhash.h"
#include "fcache_list.h"
#include "fcache.h"

#define FCACHE_BALANCE_INTERVAL 2000

struct _fcache {
    f_hash*  phash_node_index;
    fc_list* pactive_list;
    fc_list* pinactive_list;
    size_t   max_size;
    size_t   op_count;
    int      enable_balance;
};

fcache_t* fcache_create(size_t max_size)
{
    if ( !max_size ) return NULL;
    if ( max_size & 1 ) ++max_size;

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

    pcache->max_size = max_size;
    pcache->op_count = 0;
    pcache->enable_balance = 0;

    return pcache;
}

void      fcache_destroy(fcache_t* pcache)
{
    if ( !pcache ) return;
    hash_delete(pcache->phash_node_index);
    fcache_list_destory(pcache->pactive_list);
    fcache_list_destory(pcache->pinactive_list);
    free(pcache);
}

static inline
int       _fcache_add_node(fcache_t* pcache, const char* key, void* value,
                           size_t value_size)
{
    fcache_node_t* add_node = fcache_list_make_node();
    if ( !add_node ) return 1;

    add_node->data = value;

    // for a new node
    // 1. add into hash table
    // 2. add into inactive node
    hash_set_str(pcache->phash_node_index, key, add_node);
    if ( fcache_list_push(pcache->pinactive_list, add_node, value_size) ) {
        hash_del_str(pcache->phash_node_index, key);
        free(add_node);
    }

    return 0;
}

static inline
int       _fcache_update_node(fcache_t* pcache, fcache_node_t* node, void* value,
                              size_t value_size, cache_obj_free obj_free)
{
    if ( obj_free ) {
        obj_free(node->data);
    }

    node->data = value;
    fcache_list_update_node(node, value_size);

    return 0;
}

static inline
void    _fcache_del_node(fcache_t* pcache, fcache_node_t* node, const char* key,
                           cache_obj_free obj_free)
{
    size_t del_size = 0;
    if ( obj_free ) {
        obj_free(node->data);
    } else {
        fprintf(stderr, "data delete may cause memory leak\n");
    }

    hash_del_str(pcache->phash_node_index, key);
    fcache_list_delete_node(node);
    fcache_list_destory_node(node);
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
        fcache_list_push(pcache->pinactive_list, node, node->data_size);
    }

    return 0;
}

/**
 * brief check and drop nodes from inactive list
 */
static inline
void      _fcache_check_and_drop_nodes(fcache_t* pcache, fcache_node_t* node,
                                  size_t target_size, cache_obj_free obj_free)
{
    size_t result_size = 0;
    size_t current_size = fcache_list_data_size(pcache->pinactive_list) +
                          fcache_list_data_size(pcache->pactive_list);
    if ( !node ) {
        result_size = current_size + target_size;
    } else {
        diff_size = node->data_size <= target_size ? target_size - node->data_size : 0;
        result_size = current_size + diff_size;
    }

    if ( result_size > pcache->max_size ) {
        size_t drop_size = result_size - pcache->max_size;
        size_t droped_size = 0;
        while ( droped_size < drop_size ) {
            fcache_node_t* node = fcache_list_pop(pcache->pactive_list);
            if ( !node ) break;
            dropped_size += node->data_size;

            if ( obj_free ) {
                obj_free(node->data);
            }
        }
    }
}

/**
 * brief @ first incoming, we push obj into inactive list
 *       sencond incoming(fcache_get_obj trigger it), we move obj into active
 *       list if the obj has not dropped from inactive list
 *       @ if the node count will reach the max limitation, start the balance
 *       mechanism
 *       @ remove node only from inactive list
 */
int       fcache_set_obj(fcache_t* pcache, const char* key, void* value,
                         size_t value_size, cache_obj_free obj_free)
{
    if ( !pcache || !key ) return 1;
    if ( value && !value_size ) return 1;

    fcache_node_t* node = hash_get_str(pcache->phash_node_index, key);
    int ret = 0;
    if ( likely(!node) ) {
        _fcache_check_and_drop_nodes(pcache, value_size);
        ret = _fcache_add_node(pcache, key, value, value_size);
    } else {
        if ( value ) {
            ret = _fcache_update_node(pcache, node, value, value_size, obj_free);
        } else {
            _fcache_del_node(pcache, node, key, obj_free);
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
        return node->data;
    } else {
        // we don't care the result of moving, if failure, try it next time
        _fcache_move_node(pcache, node);
        return node->data;
    }
}

