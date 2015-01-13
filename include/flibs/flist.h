#ifndef FLIST_H
#define FLIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* begin;
    void* end;
} flist_iter;

typedef struct list_mgr lmgr, flist;

// return 0 continue iter.. else break out
typedef int (*flist_each_cb)(void* data);

// return <=0 : swap the node
// return > 0 : continue to scan
typedef int (*flist_compare)(void* left, void* right);

flist*     flist_create();
void       flist_delete(flist*);

int        flist_push(flist*, void* data);
void*      flist_pop(flist*);
void*      flist_head(flist*);
void*      flist_tail(flist*);
int        flist_empty(flist*);

void*      flist_foreach(flist*, flist_each_cb);
int        flist_sort(flist*, flist_compare);
flist_iter flist_new_iter(flist*);
void*      flist_each(flist_iter*);

#ifdef __cplusplus
}
#endif

#endif

