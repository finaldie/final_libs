//create by final
//desc: fhash (not thread safe)

#ifndef _HASH_H_FINAL_
#define _HASH_H_FINAL_

#ifdef __cplusplus
extern "C" {
#endif

#include "fhash_core.h"
#include "fhash_int.h"
#include "fhash_uint64.h"

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
