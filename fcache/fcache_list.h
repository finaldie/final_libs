/*
 * =====================================================================================
 *
 *       Filename:  fcache_list.h
 *
 *    Description:  fcache list
 *
 *        Version:  1.0
 *        Created:  08/17/2012 10:30:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *
 * =====================================================================================
 */

#ifndef _FCACHE_LIST_H_
#define _FCACHE_LIST_H_

typedef enum {
    INACTIVE,
    ACTIVE
} fcache_location;

typedef struct _fcache_node_t {
    void*   data;
    size_t  data_size;
    fcache_location location;
    struct _fcache_node_t* prev;
    struct _fcache_node_t* next;
} fcache_node_t;

typedef struct _fcache_list_mgr {
    fcache_node_t* head;
    fcache_node_t* tail;
} fc_list;


fc_list*        fcache_list_create();
void            fcache_list_destory(fc_list* plist);
int             fcache_list_empty(fc_list* plist);
int             fcache_list_push(fc_list* plist, fcache_node_t* node);
fcache_node_t*  fcache_list_pop(fc_list* plist);
fcache_node_t*  fcache_list_delete_node(fc_list* plist, fcache_node_t* node);
int             fcache_list_move_node(fcache_node_t* node, fc_list* from,
                                      fc_list* to);

#endif
