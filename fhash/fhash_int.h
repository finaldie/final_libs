#ifndef HASH_INT_H
#define HASH_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fhash_core.h"

typedef struct {
    // private
    fhash_iter iter;

    // read only
    void* value;
    int   key;

#if __WORDSIZE == 64
    int   padding;
#endif
} fhash_int_iter;

// foreach call back function
// return 0 to continue iterate
// return non-zero to stop iterating
typedef int (*fhash_int_each_cb)(void* ud, int key, void* value);

// hash table for: int - void*
fhash*         fhash_int_create(uint32_t init_size, uint32_t flags);
void           fhash_int_delete(fhash*);

void           fhash_int_set(fhash*, int key, void* value);
void*          fhash_int_get(fhash*, int key);
void*          fhash_int_del(fhash*, int key);

fhash_int_iter fhash_int_iter_new(fhash*);
void           fhash_int_iter_release(fhash_int_iter*);
void*          fhash_int_next(fhash_int_iter*);
void           fhash_int_foreach(fhash*, fhash_int_each_cb, void* ud);

int            fhash_int_rehash(fhash*, uint32_t new_size);

#ifdef __cplusplus
}
#endif

#endif
