#include <stdlib.h>
#include "fcache_list.h"

fc_list* fcache_list_create()
{
    fc_list* list = malloc(sizeof(fc_list));
    if ( !list ) return NULL;

    list->head = list->tail = NULL;

    return list;
}

void    fcache_list_destory(fc_list* plist)
{
    free(plist);
}

int     fcache_list_empty(fc_list* plist)
{
    if ( plist->tail == NULL && plist->head == plist->tail ) {
        return 1;
    } else {
        return 0;
    }
}

int     fcache_list_push(fc_list* plist, fcache_node_t* node)
{
    if ( !plist || !node ) return 1;

    node->next = NULL;
    node->prev = plist->tail;

    if ( fcache_list_empty(plist) ) {
        plist->head = plist->tail = node;
    } else {
        plist->tail->next = node;
        plist->tail = node;
    }
    return 0;
}

fcache_node_t* fcache_list_pop(fc_list* plist)
{
    if ( !plist || fcache_list_empty(plist) ) return NULL;

    fcache_node_t* node = plist->head;
    if ( !plist->head->next ) {
        plist->head = plist->tail = NULL;
    } else {
        plist->head = plist->head->next;
    }
    return node;
}

fcache_node_t*  fcache_list_delete_node(fc_list* plist, fcache_node_t* node)
{
    if ( !plist || !node ) return NULL;

    if ( node == plist->head && node == plist->tail ) {
        // only one node
        plist->head = plist->tail = NULL;
    } else if ( node == plist->head ) {
        plist->head = plist->head->next;
    } else if ( node == plist->tail ) {
        plist->tail = node->prev;
        plist->tail->next = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    return node;
}

int     fcache_list_move_node(fcache_node_t* node, fc_list* from, fc_list* to)
{
    if ( !node || !from ) return 1;
    fcache_list_delete_node(from, node);

    if ( to ) {
        return fcache_list_push(to, node);
    } else {
        return 0;
    }
}
