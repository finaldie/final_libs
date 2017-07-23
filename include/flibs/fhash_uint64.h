#ifndef FHASH_UINT64_H
#define FHASH_UINT64_H

#ifdef __cplusplus
extern "C" {
#endif

#include <flibs/fhash_core.h>

typedef struct {
    // private
    fhash_iter iter;

    // read only
    uint64_t   key;
    void*      value;

    // Padding for 32bit platform
    char       _padding[sizeof(void*)];
} fhash_u64_iter;

/**
 * @brief foreach call back function
 *
 * @param ud        user private data
 * @param key
 * @param value
 *
 * @return
 *                  - **0**: continue iterate
 *                  - **non-zero**: stop iterating
 */
typedef int (*fhash_u64_each_cb)(void* ud, uint64_t key, void* value);

/**
 * create fhash table
 *
 * @param init_size The hash table index size, if it's 0, the default value 10
 *                  will be used
 * @param flags
 *                  - FHASH_MASK_NONE: the default value, no feature enabled
 *                  - FHASH_MASK_AUTO_REHASH: enable auto-rehash (recommended)
 *
 * @return          fhash table pointer
 */
fhash*         fhash_u64_create(uint32_t init_size, uint32_t flags);

/**
 * @brief destroy a fhash table
 *
 * @param table     pointer of fhash table
 * @return          void
 */
void           fhash_u64_delete(fhash* table);

/**
 * @brief add or set a key-value pair into fhash table
 *
 * @param table     pointer of fhash table
 * @param key       key
 * @param value     value
 *
 * @return          void
 */
void           fhash_u64_set(fhash* table, uint64_t key, const void* value);

/**
 * @brief get the value of the key
 *
 * @param table     pointer of fhash table
 * @param key       key
 *
 * @return          return value's pointer
 */
void*          fhash_u64_get(fhash* table, uint64_t key);

/**
 * @brief delete a key-value pair from the fhash table
 *
 * @param table     pointer of fhash table
 * @param key       key
 *
 * @return          void
 */
void*          fhash_u64_del(fhash* table, uint64_t key);

/**
 * @brief create a iterator of the fhash table
 *
 * @param table     pointer of fhash table
 * @return          a iterator of this table
 */
fhash_u64_iter fhash_u64_iter_new(fhash* table);

/**
 * @brief release a iterator of the fhash table
 *
 * @param iter     a pointer of iterator
 * @return         void
 *
 * @note           user must call this api if the iteration operation is done
 */
void           fhash_u64_iter_release(fhash_u64_iter* iter);

/**
 * @brief get the next element
 *
 * @param iter     pointer of the iterator
 * @return
 *                 - the next element
 *                 - NULL if reach the end
 */
void*          fhash_u64_next(fhash_u64_iter* iter);

/**
 * @brief another iteration way
 *
 * @param table    pointer of fhash table
 * @param cb       the callback function of the iteration
 * @param ud       user private data, it will be passed to the callback
 *                 function
 *
 * @return         void
 */
void           fhash_u64_foreach(fhash* table, fhash_u64_each_cb cb, void* ud);

#ifdef __cplusplus
}
#endif

#endif

