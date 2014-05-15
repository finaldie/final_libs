#ifndef HASH_STR_H
#define HASH_STR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fhash_core.h"

typedef struct {
    // private
    fhash_iter iter;

    // read only
    const char* key;
    void* value;
} fhash_str_iter;

// foreach call back function
// return 0 to continue iterate
// return non-zero to stop iterating
typedef int (*fhash_str_each_cb)(void* ud, const char* key, void* value);

// hash table for: string - void*
fhash*         fhash_str_create(uint32_t init_size, uint32_t flags);
void           fhash_str_delete(fhash*);

void           fhash_str_set(fhash*, const char* key, void* value);
void*          fhash_str_get(fhash*, const char* key);
void*          fhash_str_del(fhash*, const char* key);

fhash_str_iter fhash_str_iter_new(fhash*);
void           fhash_str_iter_release(fhash_str_iter*);
void*          fhash_str_next(fhash_str_iter*);
void           fhash_str_foreach(fhash*, fhash_str_each_cb, void* ud);

#ifdef __cplusplus
}
#endif

#endif
