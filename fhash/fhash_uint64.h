#ifndef HASH_UINT64_H
#define HASH_UINT64_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fhash_core.h"

typedef struct {
    // private
    fhash_iter iter;

    // read only
    uint64_t key;
    void* value;
} fhash_u64_iter;

// foreach call back function
// return 0 to continue iterate
// return non-zero to stop iterating
typedef int (*fhash_u64_each_cb)(void* ud, uint64_t key, void* value);

// hash table for: uint64 - void*
fhash*         fhash_u64_create(uint32_t init_size, uint32_t flags);
void           fhash_u64_delete(fhash*);

void           fhash_u64_set(fhash*, uint64_t key, void* value);
void*          fhash_u64_get(fhash*, uint64_t key);
void*          fhash_u64_del(fhash*, uint64_t key);

fhash_u64_iter fhash_u64_iter_new(fhash*);
void           fhash_u64_iter_release(fhash_u64_iter*);
void*          fhash_u64_next(fhash_u64_iter*);
void           fhash_u64_foreach(fhash*, fhash_u64_each_cb, void* ud);

#ifdef __cplusplus
}
#endif

#endif
