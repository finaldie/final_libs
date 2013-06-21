//base info: create by hyz
/*effect: lockfree fifo list
*
*
*/


#ifndef _LIST_H_
#define _LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* begin;
    void* end;
} liter;

typedef struct list_mgr lmgr, *pl_mgr;

// return 0 continue iter.. else break out
typedef int (*plist_call_back)(void* data);

typedef int (*compare)(void* a, void* b);

pl_mgr  flist_create();
void    flist_delete(pl_mgr);

int     flist_push(pl_mgr, void* data);
void*   flist_pop(pl_mgr);
void*   flist_head(pl_mgr);
int     flist_isempty(pl_mgr);

void*   flist_foreach(pl_mgr, plist_call_back);
void* flist_sort(pl_mgr, compare );
liter   flist_iter(pl_mgr);
void*   flist_each(liter*);

#ifdef __cplusplus
}
#endif

#endif
