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
    size_t   current_size;
    size_t   active_nodes_count;
    size_t   inactive_nodes_count;
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
    pcache->current_size = 0;
    pcache->active_nodes_count = 0;
    pcache->inactive_nodes_count = 0;
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
    fcache_node_t* add_node = malloc(sizeof(fcache_node_t));
    if ( !add_node ) return 1;

    add_node->data = value;
    add_node->data_size = value_size;
    add_node->location = INACTIVE;
    add_node->prev = add_node->next = NULL;

    // for a new node
    // 1. add into hash table
    // 2. add into inactive node
    hash_set_str(pcache->phash_node_index, key, add_node);
    if ( !fcache_list_push(pcache->pinactive_list, add_node) ) {
        pcache->current_size += value_size;
        pcache->inactive_nodes_count++;
    } else {
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
        pcache->current_size -= node->data_size;
        node->data_size = 0;
    }

    node->data = value;
    node->data_size = value_size;
    pcache->current_size += node->data_size;

    return 0;
}

static inline
void      _fcache_del_node(fcache_t* pcache, fcache_node_t* node, const char* key,
                           cache_obj_free obj_free)
{
    if ( obj_free ) {
        obj_free(node->data);
        pcache->current_size -= node->data_size;
        node->data_size = 0;
    } else {
        fprintf(stderr, "data delete may cause memory leak\n");
    }

    if ( node->location == INACTIVE ) {
        fcache_list_delete_node(pcache->pinactive_list, node);
        pcache->inactive_nodes_count--;
    } else {
        fcache_list_delete_node(pcache->pactive_list, node);
        pcache->active_nodes_count--;
    }

    hash_del_str(pcache->phash_node_index, key);
    free(node);
}

static inline
int       _fcache_move_node(fcache_t* pcache, fcache_node_t* node)
{
    fcache_list_move_node(node, pcache->pinactive_list,
                                 pcache->pactive_list);
    node->location = ACTIVE;
    pcache->inactive_nodes_count--;
    pcache->active_nodes_count++;

    return 0;
}

static inline
int       _fcache_balance_nodes(fcache_t* pcache)
{
    size_t avg_length = ( pcache->inactive_nodes_count + pcache->active_nodes_count ) / 2;
    if ( pcache->active_nodes_count <= avg_length ) {
        return 0;
    }

    size_t need_move_count = pcache->active_nodes_count - avg_length;
    size_t i = 0;
    for ( ; i < need_move_count; i++ ) {
        fcache_node_t* node = fcache_list_pop(pcache->pactive_list);
        if ( !node ) break;

        pcache->active_nodes_count--;
        if ( !fcache_list_push(pcache->pinactive_list, node) ) {
            pcache->inactive_nodes_count++;
        }
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
int       fcache_add_obj(fcache_t* pcache, const char* key, void* value,
                         size_t value_size, cache_obj_free obj_free)
{
    if ( !pcache || !key ) return 1;
    if ( value && !value_size ) return 1;

    fcache_node_t* node = hash_get_str(pcache->phash_node_index, key);
    int ret = 0;
    if ( likely(!node) ) {
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
        size_t avg_node_size = pcache->current_size /
            ( pcache->inactive_nodes_count + pcache->active_nodes_count );
        size_t times = (pcache->max_size - pcache->current_size) / avg_node_size;

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

    if ( node->location == ACTIVE ) {
        return node->data;
    } else {
        // we don't care the result of moving, if failure, try it next time
        _fcache_move_node(pcache, node);
        return node->data;
    }
}


