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

flist*  flist_create()
{
    flist* pmgr = (flist*)malloc(sizeof(lmgr));
    pnode  node = (pnode)malloc(sizeof(lnode));
    node->pre = node->next = NULL;
    pmgr->head = pmgr->tail = node;

    return pmgr;
}

void    flist_delete(flist* pmgr)
{
    if ( !pmgr ) return;
    while( flist_pop(pmgr) );
    free(pmgr->head);
    free(pmgr);
}

int     flist_push(flist* pmgr, void* data)
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

void*   flist_pop(flist* pmgr)
{
    if ( !pmgr || flist_empty(pmgr) )
        return NULL;

    pmgr->head = pmgr->head->next;
    free(pmgr->head->pre);
    return pmgr->head->data;
}

int     flist_empty(flist* pmgr)
{
    if (pmgr->head == pmgr->tail)
        return 1;
    return 0;
}

void*   flist_foreach(flist* pmgr, flist_each_cb pfunc)
{
    if ( !pfunc ) return NULL;
    if ( flist_empty(pmgr) ) return NULL;

    pnode node = LIST_HEAD(pmgr)->next;
    while ( node ) {
        if ( pfunc(node->data) )
            break;

        node = node->next;
    }

    return NULL;
}

//insertion sort, sort list in increasing order
int flist_sort(flist* pmgr, flist_compare pfunc)
{
    if ( !pfunc ) return 1;
    if ( flist_empty(pmgr) ) return 1;

    //init node_i at 2nd data struct
    pnode node_i = LIST_HEAD(pmgr)->next->next;
    pnode node_j, temp;

    //insert node_i into sorted list each time
    while(node_i)
    {
        node_j = node_i->pre;
        while(node_j != LIST_HEAD(pmgr))
        {
            //find 1st position which is le node_i
            if(pfunc(node_j->data, node_i->data) <= 0)
                break;
            node_j = node_j->pre;
        }

        if(node_j != node_i->pre)
        {
            //update tail ptr before delete node_i
            if(node_i == LIST_TAIL(pmgr))
                LIST_TAIL(pmgr) = node_i->pre;

            //delete node_i
            temp = node_i;
            temp->pre->next = temp->next;
            if(temp->next)
                temp->next->pre = temp->pre;

            //insert temp after node_j
            temp->next = node_j->next;
            temp->pre = node_j;
            node_j->next = temp;
            temp->next->pre = temp;
        }
        node_i = node_i->next;
    }
    return 0;
}

inline
flist_iter   flist_new_iter(flist* pmgr)
{
    flist_iter iter;

    iter.begin = (void*)pmgr->head;
    iter.end = (void*)pmgr->tail;

    return iter;
}

void*   flist_each(flist_iter* iter)
{
    flist* pmgr = (flist*)iter;
    if ( flist_empty(pmgr) ) return NULL;

    pnode node = LIST_HEAD(pmgr)->next;

    if( !node ) return NULL;

    void* data = node->data;
    LIST_HEAD(pmgr) = node;

    return data;
}

void*   flist_head(flist* pmgr){
    if ( flist_empty(pmgr) ) return NULL;

    return LIST_HEAD(pmgr)->next->data;
}

void*   flist_tail(flist* pmgr){
    if ( flist_empty(pmgr) ) return NULL;

    return LIST_TAIL(pmgr)->data;
}
