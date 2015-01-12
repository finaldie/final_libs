#include <stdlib.h>
#include <string.h>
#include "flibs/fdlist.h"

#define FO_PREV(orig_node)     ((orig_node)->priv.prev)
#define FO_NEXT(orig_node)     ((orig_node)->priv.next)
#define FO_OWNER(orig_node)    ((orig_node)->priv.owner)
#define FO_DATASIZE(orig_node) ((orig_node)->priv.data_size)
#define FN_HEAD(node)          (fdlist_orig_node_t*)((fdlist_priv_t*)node - 1)
#define FN_PREV(node)          (FO_PREV(FN_HEAD(node)))
#define FN_NEXT(node)          (FO_NEXT(FN_HEAD(node)))
#define FN_OWNER(node)         (FO_OWNER(FN_HEAD(node)))
#define FN_DATASIZE(node)      (FO_DATASIZE(FN_HEAD(node)))

#pragma pack(4)
struct fdlist_orig_node_t;
typedef struct fdlist_priv_t {
    struct fdlist_orig_node_t* prev;
    struct fdlist_orig_node_t* next;
    fdlist*                    owner;
    size_t                     data_size;
} fdlist_priv_t;

struct _fdlist_node_t {
    void*  data;
};

typedef struct fdlist_orig_node_t {
    // private data
    fdlist_priv_t priv;

    // user data
    fdlist_node_t node;
} fdlist_orig_node_t;

struct _fdlist_mgr {
    fdlist_orig_node_t* head;
    fdlist_orig_node_t* tail;
    size_t size;
    size_t total_data_size;
};
#pragma pack()

fdlist* fdlist_create()
{
    fdlist* list = malloc(sizeof(fdlist));
    if ( !list ) return NULL;

    list->head = list->tail = NULL;
    list->size = 0;
    list->total_data_size = 0;

    return list;
}

void     fdlist_destroy(fdlist* plist)
{
    free(plist);
}

int      fdlist_empty(fdlist* plist)
{
    if ( plist->tail == NULL && plist->head == plist->tail && !plist->size ) {
        return 1;
    } else {
        return 0;
    }
}

fdlist_node_t* fdlist_make_node(void* data, size_t data_size)
{
    if( !data || !data_size ) {
        return NULL;
    }

    fdlist_orig_node_t* orig_node = malloc(sizeof(fdlist_orig_node_t));
    if ( !orig_node ) return NULL;
    memset(orig_node, 0, sizeof(fdlist_orig_node_t));

    if( fdlist_set_nodedata(&orig_node->node, data, data_size) ) {
        free(orig_node);
        return NULL;
    }

    return &orig_node->node;
}

void     fdlist_destroy_node(fdlist_node_t* node)
{
    free(FN_HEAD(node));
}

int      fdlist_push(fdlist* plist, fdlist_node_t* node)
{
    if ( !plist || !node ) return 1;

    FN_NEXT(node) = NULL;
    FN_PREV(node) = plist->tail;
    FN_OWNER(node) = plist;

    if ( fdlist_empty(plist) ) {
        plist->head = plist->tail = FN_HEAD(node);
    } else {
        FO_NEXT(plist->tail) = FN_HEAD(node);
        plist->tail = FN_HEAD(node);
    }
    plist->size++;
    plist->total_data_size += FN_DATASIZE(node);
    return 0;
}

fdlist_node_t* fdlist_pop(fdlist* plist)
{
    if ( !plist || fdlist_empty(plist) ) return NULL;

    fdlist_orig_node_t* orig_node = plist->head;
    if ( !FO_NEXT(plist->head) ) {
        plist->head = plist->tail = NULL;
    } else {
        plist->head = FO_NEXT(plist->head);
    }

    FO_PREV(orig_node) = FO_NEXT(orig_node) = NULL;
    FO_OWNER(orig_node) = NULL;
    plist->size--;
    plist->total_data_size -= FO_DATASIZE(orig_node);
    return &orig_node->node;
}

fdlist_node_t* fdlist_delete_node(fdlist_node_t* node)
{
    if ( !node ) return NULL;
    if ( !FN_OWNER(node) ) return NULL;

    fdlist* plist = FN_OWNER(node);
    fdlist_orig_node_t* orig_node = FN_HEAD(node);
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
    FN_PREV(node) = FN_NEXT(node) = NULL;
    FN_OWNER(node) = NULL;
    plist->size--;
    plist->total_data_size -= FO_DATASIZE(orig_node);
    return node;
}

int      fdlist_move_node(fdlist_node_t* node, fdlist* to)
{
    if ( !node ) return 1;

    fdlist* from = FN_OWNER(node);
    // if size == 1, don't need to move it
    if ( from == to && from->size == 1 ) {
        return 0;
    }

    fdlist_delete_node(node);
    if ( to ) {
        return fdlist_push(to, node);
    } else {
        return 0;
    }
}

static inline
void     fdlist_update_node_size(fdlist_node_t* node, size_t size)
{
    if ( !node ) return;
    FN_DATASIZE(node) = size;

    fdlist* plist = FN_OWNER(node);
    if( plist ) {
        plist->total_data_size -= FN_DATASIZE(node);
        plist->total_data_size += size;
    }
}

size_t   fdlist_size(fdlist* plist)
{
    if ( !plist ) return 0;
    return plist->size;
}

size_t   fdlist_data_size(fdlist* plist)
{
    if ( !plist ) return 0;
    return plist->total_data_size;
}

size_t   fdlist_node_size(fdlist_node_t* node)
{
    if ( node ) {
        return FN_DATASIZE(node);
    } else {
        return 0;
    }
}

fdlist* fdlist_node_owner(fdlist_node_t* node)
{
    if ( node ) {
        return FN_OWNER(node);
    } else {
        return NULL;
    }
}

int      fdlist_set_nodedata(fdlist_node_t* node, void* data, size_t data_size)
{
    if ( !node || !data ) return 1;
    node->data = data;
    fdlist_update_node_size(node, data_size);
    return 0;
}

void*    fdlist_get_nodedata(fdlist_node_t* node)
{
    if ( !node ) return NULL;
    return node->data;
}

fdlist_node_t* fdlist_foreach(fdlist* plist, fdlist_each_cb each_cb)
{
    if( !plist || !each_cb ) {
        return NULL;
    }

    if ( fdlist_empty(plist) ) {
        return NULL;
    }

    fdlist_orig_node_t* iter_node = plist->head;
    while( !iter_node ) {
        if( each_cb(&iter_node->node) ) {
            return &iter_node->node;
        }

        iter_node = FO_NEXT(iter_node);
    }

    return NULL;
}
