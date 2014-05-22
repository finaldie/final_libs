#ifndef FHASH_CORE_H
#define FHASH_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

// fhash flags
#define FHASH_MASK_NONE         0x0
#define FHASH_MASK_AUTO_REHASH  0x1

// fhash_profile flags
#define FHASH_PROF_SILENT       0x0
#define FHASH_PROF_VERBOSE      0x1

typedef int32_t key_sz_t;
typedef int64_t value_sz_t;

typedef struct fhash fhash;

// hash table operation functions, all the function will be called
// on demand by hash table itself
typedef struct {
    uint32_t (*hash_alg) (const void* key,  key_sz_t key_sz);

    // return 0: key1 is same as key2
    // return non-zero: key1 is different with key2
    int      (*compare)  (const void* key1, key_sz_t key_sz1,
                          const void* key2, key_sz_t key_sz2);
} fhash_opt;

typedef struct {
    // private
    fhash*   phash;
    void*    node;
    size_t   slot;    // the last location of node slot
    uint32_t index;   // the last location of hash table index

    // read only
    key_sz_t    key_sz;
    value_sz_t  value_sz;
    const void* key;
    void*       value;
} fhash_iter;

// foreach call back function
// return 0 to continue iterate
// return non-zero to stop iterating
typedef int (*fhash_each_cb)(void* ud,
               const void* key, key_sz_t key_sz,
               void* value, value_sz_t value_sz);

// APIs
fhash*     fhash_create(uint32_t init_size, fhash_opt, void* ud,
                        uint32_t flags);
void       fhash_delete(fhash*);

void       fhash_set(fhash*,
               const void* key, key_sz_t key_sz,
               const void* value, value_sz_t value_sz);
void*      fhash_get(fhash*, const void* key, key_sz_t key_sz,
               value_sz_t* value_sz);
void       fhash_del(fhash*, const void* key, key_sz_t key_sz);
void*      fhash_fetch_and_del(fhash* phash,
               const void* key, key_sz_t key_sz,
               void* value, value_sz_t value_sz);

fhash_iter fhash_iter_new(fhash*);
void       fhash_iter_release(fhash_iter*);
void*      fhash_next(fhash_iter*);
void       fhash_foreach(fhash*, fhash_each_cb, void* ud);

int        fhash_rehash(fhash*, uint32_t new_size);

// profiling
typedef struct fhash_profile_data {
    size_t   total_slots;
    size_t   used_slots;
    uint32_t index_size;
    uint32_t index_used;
} fhash_profile_data;

void       fhash_profile(fhash*, fhash_profile_data*, int flags);

#ifdef __cplusplus
}
#endif

#endif
