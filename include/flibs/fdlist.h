#ifndef FD_LIST_H
#define FD_LIST_H

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
fdlist_node_t*  fdlist_make_node(const void* data, size_t data_size);
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
int             fdlist_set_nodedata(fdlist_node_t*, const void* d, size_t size);
void*           fdlist_get_nodedata(fdlist_node_t*);

#ifdef __cplusplus
}
#endif

#endif

