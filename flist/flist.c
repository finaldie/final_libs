//base info: create by hyz
//effect: fifo list

#include <stdio.h>
#include <stdlib.h>

#include "flist.h"

#define LIST_HEAD(pmgr) (pmgr->head)
#define LIST_TAIL(pmgr) (pmgr->tail)

struct list_node {
    void*             data;
    struct list_node* pre;
    struct list_node* next;
};

typedef struct list_node lnode, *pnode;

struct list_mgr {
    struct list_node* head;
    struct list_node* tail;
};

pl_mgr  flist_create()
{
    pl_mgr pmgr = (pl_mgr)malloc(sizeof(lmgr));
    pnode  node = (pnode)malloc(sizeof(lnode));
    node->pre = node->next = NULL;
    pmgr->head = pmgr->tail = node;

    return pmgr;
}

void    flist_delete(pl_mgr pmgr)
{
    while( flist_pop(pmgr) );
    free(pmgr->head);
    free(pmgr);
}

int     flist_push(pl_mgr pmgr, void* data)
{
    if ( !data )
        return 1;

    pnode node = (pnode)malloc(sizeof(lnode));
    node->data = data;
    node->next = NULL;
    node->pre  = pmgr->tail;
    pmgr->tail->next = node;
    pmgr->tail = node;

    return 0;
}

void*   flist_pop(pl_mgr pmgr)
{
    if ( !pmgr || flist_isempty(pmgr) )
        return NULL;

    pmgr->head = pmgr->head->next;
    free(pmgr->head->pre);
    return pmgr->head->data;
}

int     flist_isempty(pl_mgr pmgr)
{
    if (pmgr->head == pmgr->tail)
        return 1;
    return 0;
}

void*   flist_foreach(pl_mgr pmgr, plist_call_back pfunc)
{
    if ( !pfunc ) return NULL;
    if ( flist_isempty(pmgr) ) return NULL;

    pnode node = LIST_HEAD(pmgr)->next;
    while ( node ) {
        if ( pfunc(node->data) )
            break;

        node = node->next;
    }

    return NULL;
}

inline
liter   flist_iter(pl_mgr pmgr)
{
    liter iter;

    iter.begin = (void*)pmgr->head;
    iter.end = (void*)pmgr->tail;

    return iter;
}

void*   flist_each(liter* iter)
{
    pl_mgr pmgr = (pl_mgr)iter;
    if ( flist_isempty(pmgr) ) return NULL;

    pnode node = LIST_HEAD(pmgr)->next;

    if( !node ) return NULL;

    void* data = node->data;
    LIST_HEAD(pmgr) = node;

    return data;
}

void*   flist_head(pl_mgr pmgr){
    if ( flist_isempty(pmgr) ) return NULL;

    return LIST_HEAD(pmgr)->next->data;
}
