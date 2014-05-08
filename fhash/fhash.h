//create by final
//desc: fhash (not thread safe)

#ifndef _HASH_H_FINAL_
#define _HASH_H_FINAL_

#ifdef __cplusplus
extern "C" {
#endif

#include "fhash_core.h"
#include "fhash_int.h"

// hash table for: uint64 - void*
fhash* fhash_uint64_create(uint32_t init_size, void* ud, uint32_t flags);
void   fhash_uint64_delete(fhash*);

void   fhash_uint64_set(fhash*, uint64_t key, void* value);
void*  fhash_uint64_get(fhash*, uint64_t key);
void*  fhash_uint64_del(fhash*, uint64_t key);

// hash table for: string - void*
fhash* fhash_str_create(uint32_t init_size, void* ud, uint32_t flags);
void   fhash_str_delete(fhash*);

void   fhash_str_set(fhash*, const void* key, void* value);
void*  fhash_str_get(fhash*, const void* key);
void*  fhash_str_del(fhash*, const void* key);

#ifdef __cplusplus
}
#endif

#endif
