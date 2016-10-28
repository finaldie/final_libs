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

typedef int32_t key_sz_t;
typedef size_t  value_sz_t;

typedef struct fhash fhash;

/**
 * hash table operation functions, all the function will be called
 * on demand by hash table itself
 */
typedef struct {
    uint32_t (*hash_alg) (const void* key,  key_sz_t key_sz);

    /**
     * return
     *   - **0**: key1 is same as key2
     *   - **non-zero**: key1 is different with key2
     */
    int      (*compare)  (const void* key1, key_sz_t key_sz1,
                          const void* key2, key_sz_t key_sz2);
} fhash_opt;

/**
 * fhash iterator
 *
 * @note DO NOT modify any member's data, user only need to read the data of
 *       **read only** part
 */
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

/**
 * foreach call back function
 * @return
 *   - **0**: continue iterate
 *   - **non-zero**: stop iterating
 */
typedef int (*fhash_each_cb)(void* ud,
               const void* key, key_sz_t key_sz,
               void* value, value_sz_t value_sz);

// APIs

/**
 * @brief create a fhash table
 *
 * @param init_size The hash table index size, if it's 0, the default value 10
 *                  will be used
 * @param opt       The operation structure which contain hash algorithm and
 *                  comparison call back functions
 * @param flags
 *                  - FHASH_MASK_NONE: the default value, no feature enabled
 *                  - FHASH_MASK_AUTO_REHASH: enable auto-rehash (recommended)
 *
 * @return          fhash table pointer
 */
fhash*     fhash_create(uint32_t init_size, fhash_opt opt, uint32_t flags);

/**
 * @brief destroy a fhash table
 *
 * @param table     pointer of fhash table
 * @return          void
 */
void       fhash_delete(fhash* table);

/**
 * @brief add or set a key-value pair into fhash table
 *
 * @param table     pointer of fhash table
 * @param key       key
 * @param key_sz    key size
 * @param value     value
 * @param value_sz  value size
 * @return          void
 */
void       fhash_set(fhash* table,
               const void* key, key_sz_t key_sz,
               const void* value, value_sz_t value_sz);

/**
 * @brief get the value of the key
 *
 * @param table     pointer of fhash table
 * @param key       key
 * @param key_sz    key size
 * @param value_sz  a output variable, which is used for storing the value's
 *                  size if value_sz is not NULL
 * @return          return value's pointer
 */
void*      fhash_get(fhash* table, const void* key, key_sz_t key_sz,
               value_sz_t* value_sz);

/**
 * @brief delete a key-value pair from the fhash table
 *
 * @param table     pointer of fhash table
 * @param key       key
 * @param key_sz    key size
 * @return          void
 */
void       fhash_del(fhash* table, const void* key, key_sz_t key_sz);

/**
 * @brief fetch the key's value then delete it from fhash table
 *
 * @param table     pointer of fhash table
 * @param key       key
 * @param key_sz    key size
 * @param value     value
 * @param value_sz  value size
 * @return
 *                  - key's value if the key is exist
 *                  - NULL if the key is non-exist
 */
void*      fhash_fetch_and_del(fhash* table,
               const void* key, key_sz_t key_sz,
               void* value, value_sz_t value_sz);

/**
 * @brief create a iterator of the fhash table
 *
 * @param table     pointer of fhash table
 * @return          a iterator of this table
 */
fhash_iter fhash_iter_new(fhash* table);

/**
 * @brief release a iterator of the fhash table
 *
 * @param iter     a pointer of iterator
 * @return         void
 *
 * @note           user must call this api if the iteration operation is done
 */
void       fhash_iter_release(fhash_iter* iter);

/**
 * @brief get the next element
 *
 * @param iter     pointer of the iterator
 * @return
 *                 - the next element
 *                 - NULL if reach the end
 */
void*      fhash_next(fhash_iter* iter);

/**
 * @brief another iteration way
 *
 * @param table    pointer of fhash table
 * @param cb       the callback function of the iteration
 * @param ud       user private data, it will be passed to the callback
 *                 function
 * @return         void
 */
void       fhash_foreach(fhash* table, fhash_each_cb cb, void* ud);

/**
 * @brief do the re-hash operation
 *
 * @param table    pointer of fhash table
 * @param new_size the new index size for this rehash operation
 * @return
 *                 - **0**: if operation is successful
 *                 - **1**: if the operation is failed
 *
 * @note           generally, the rehash will fail if:
 *                 -# there are still some iteration operations ongoing
 *                 -# the index size has already reach to UINT32_MAX
 *                 -# the new_size parameter is equal to current index size
 */
int        fhash_rehash(fhash* table, uint32_t new_size);


// fhash_profile flags
#define FHASH_PROF_SILENT       0x0
#define FHASH_PROF_VERBOSE      0x1

typedef struct fhash_profile_data {
    size_t   total_slots;
    size_t   used_slots;
    uint32_t index_size;
    uint32_t index_used;
} fhash_profile_data;

/**
 * @brief profile the hash table, fill the profiling data to the
 *        fhash_profile_data structure
 *
 * @param table    pointer of fhash table
 * @param flags
 *                 - FHASH_PROF_SILENT: won't print to stdout, only fill data
 *                 - FHASH_PROF_VERBOSE: will print more detail to stdout
 *
 * @return         void
 */
void       fhash_profile(fhash* table, int flags, fhash_profile_data* data);

#ifdef __cplusplus
}
#endif

#endif

