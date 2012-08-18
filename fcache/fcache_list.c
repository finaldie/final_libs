#include <stdlib.h>
#include <string.h>
#include "fcache_list.h"

#define FO_PREV(orig_node)     ((orig_node)->priv.prev)
#define FO_NEXT(orig_node)     ((orig_node)->priv.next)
#define FO_OWNER(orig_node)    ((orig_node)->priv.owner)
#define FO_DATASIZE(orig_node) ((orig_node)->priv.data_size)
#define FN_HEAD(node)          (fc_orig_node_t*)((fc_priv_t*)node - 1)
#define FN_PREV(node)          (FO_PREV(FN_HEAD(node)))
#define FN_NEXT(node)          (FO_NEXT(FN_HEAD(node)))
#define FN_OWNER(node)         (FO_OWNER(FN_HEAD(node)))
#define FN_DATASIZE(node)      (FO_DATASIZE(FN_HEAD(node)))

#pragma pack(4)
struct fcache_orig_node_t;
typedef struct fcache_priv_t {
    struct fcache_orig_node_t* prev;
    struct fcache_orig_node_t* next;
    fc_list*               owner;
    size_t                 data_size;
} fc_priv_t;

typedef struct fcache_orig_node_t {
    // private data
    fc_priv_t     priv;

    // user data
    fcache_node_t node;
} fc_orig_node_t;

struct _fcache_list_mgr {
    fc_orig_node_t* head;
    fc_orig_node_t* tail;
    size_t size;
    size_t total_data_size;
};
#pragma pack()

fc_list* fcache_list_create()
{
    fc_list* list = malloc(sizeof(fc_list));
    if ( !list ) return NULL;

    list->head = list->tail = NULL;
    list->size = 0;
    list->total_data_size = 0;

    return list;
}

void    fcache_list_destory(fc_list* plist)
{
    free(plist);
}

int     fcache_list_empty(fc_list* plist)
{
    if ( plist->tail == NULL && plist->head == plist->tail && !plist->size ) {
        return 1;
    } else {
        return 0;
    }
}

fcache_node_t*  fcache_list_make_node()
{
    fc_orig_node_t* orig_node = malloc(sizeof(fc_orig_node_t));
    if ( !orig_node ) return NULL;

    memset(orig_node, 0, sizeof(fc_orig_node_t));
    return &orig_node->node;
}

void    fcache_list_destory_node(fcache_node_t* node)
{
    free(FN_HEAD(node));
}

int     fcache_list_push(fc_list* plist, fcache_node_t* node, size_t data_size)
{
    if ( !plist || !node ) return 1;

    FN_NEXT(node) = NULL;
    FN_PREV(node) = plist->tail;
    FN_OWNER(node) = plist;
    FN_DATASIZE(node) = data_size;

    if ( fcache_list_empty(plist) ) {
        plist->head = plist->tail = FN_HEAD(node);
    } else {
        FO_NEXT(plist->tail) = FN_HEAD(node);
        plist->tail = FN_HEAD(node);
    }
    plist->size++;
    plist->total_data_size += FN_DATASIZE(node);
    return 0;
}

fcache_node_t* fcache_list_pop(fc_list* plist)
{
    if ( !plist || fcache_list_empty(plist) ) return NULL;

    fc_orig_node_t* orig_node = plist->head;
    if ( !FO_NEXT(plist->head) ) {
        plist->head = plist->tail = NULL;
    } else {
        plist->head = FO_NEXT(plist->head);
    }
    plist->size--;
    plist->total_data_size -= FO_DATASIZE(orig_node);
    return &orig_node->node;
}

fcache_node_t*  fcache_list_delete_node(fcache_node_t* node)
{
    if ( !node ) return NULL;

    fc_list* plist = FN_OWNER(node);
    fc_orig_node_t* orig_node = FN_HEAD(node);
    if ( orig_node == plist->head && orig_node == plist->tail ) {
        // only one node
        plist->head = plist->tail = NULL;
    } else if ( orig_node == plist->head ) {
        plist->head = FO_NEXT(plist->head);
    } else if ( orig_node == plist->tail ) {
        plist->tail = FO_PREV(orig_node);
        FO_NEXT(plist->tail) = NULL;
    } else {
        FO_NEXT(FO_PREV(orig_node)) = FO_NEXT(orig_node);
        FO_PREV(FO_NEXT(orig_node)) = FO_PREV(orig_node);
    }
    plist->size--;
    plist->total_data_size -= FO_DATASIZE(orig_node);
    return node;
}

int     fcache_list_move_node(fcache_node_t* node, fc_list* to)
{
    if ( !node ) return 1;

    fc_list* from = FN_OWNER(node);
    // if size == 1, don't need to move it
    if ( from == to && from->size == 1 ) {
        return 0;
    }

    fcache_list_delete_node(node);
    if ( to ) {
        return fcache_list_push(to, node, FN_DATASIZE(node));
    } else {
        return 0;
    }
}

void    fcache_list_update_node(fcache_node_t* node, size_t size)
{
    if ( !node ) return;

    fc_list* plist = FN_OWNER(node);
    plist->total_data_size -= FN_DATASIZE(node);
    plist->total_data_size += size;
}

size_t  fcache_list_size(fc_list* plist)
{
    if ( !plist ) return 0;
    return plist->size;
}

size_t  fcache_list_data_size(fc_list* plist)
{
    if ( !plist ) return 0;
    return plist->total_data_size;
}

size_t  fcache_list_node_size(fcache_node_t* node)
{
    if ( node ) {
        return FN_DATASIZE(node);
    } else {
        return 0;
    }
}

fc_list* fcache_list_node_owner(fcache_node_t* node)
{
    if ( node ) {
        return FN_OWNER(node);
    } else {
        return NULL;
    }
}
