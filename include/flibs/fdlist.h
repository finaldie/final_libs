/*
 * =============================================================================
 *
 *       Filename:  fdlist.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/25/2013 12:40:55 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:
 *
 * =============================================================================
 */

#ifndef _FD_LIST_H_
#define _FD_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct _fdlist_node_t fdlist_node_t;
typedef struct _fdlist_mgr    fdlist;

// fdlist_foreach callback
// return 0, continue iterating
// return non-zero, stop iterating
typedef int (*fdlist_each_cb)(fdlist_node_t*);

fdlist*         fdlist_create();
void            fdlist_destroy(fdlist* plist);
int             fdlist_empty(fdlist* plist);
fdlist_node_t*  fdlist_make_node(void* data, size_t data_size);
void            fdlist_destroy_node(fdlist_node_t* node);
int             fdlist_push(fdlist* plist, fdlist_node_t* node);
fdlist_node_t*  fdlist_pop(fdlist* plist);
fdlist_node_t*  fdlist_delete_node(fdlist_node_t* node);
int             fdlist_move_node(fdlist_node_t* node, fdlist* to);
fdlist_node_t*  fdlist_foreach(fdlist* plist, fdlist_each_cb);
size_t          fdlist_size(fdlist* plist);
size_t          fdlist_data_size(fdlist* plist);
size_t          fdlist_node_size(fdlist_node_t* node);
fdlist*         fdlist_node_owner(fdlist_node_t* node);
int             fdlist_set_nodedata(fdlist_node_t*, void* data, size_t size);
void*           fdlist_get_nodedata(fdlist_node_t*);

#ifdef __cplusplus
}
#endif

#endif
