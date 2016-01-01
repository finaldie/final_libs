#ifndef FHASH_INT_H
#define FHASH_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <flibs/fhash_core.h>

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
typedef int (*fhash_int_each_cb) (void* ud, int key, void* value);

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
fhash*         fhash_int_create(uint32_t init_size, uint32_t flags);

/**
 * @brief destroy a fhash table
 *
 * @param table     pointer of fhash table
 * @return          void
 */
void           fhash_int_delete(fhash* table);

/**
 * @brief add or set a key-value pair into fhash table
 *
 * @param table     pointer of fhash table
 * @param key       key
 * @param value     value
 *
 * @return          void
 */
void           fhash_int_set(fhash* table, int key, const void* value);

/**
 * @brief get the value of the key
 *
 * @param table     pointer of fhash table
 * @param key       key
 *
 * @return          return value's pointer
 */
void*          fhash_int_get(fhash* table, int key);

/**
 * @brief delete a key-value pair from the fhash table
 *
 * @param table     pointer of fhash table
 * @param key       key
 *
 * @return          void
 */
void*          fhash_int_del(fhash* table, int key);

/**
 * @brief create a iterator of the fhash table
 *
 * @param table     pointer of fhash table
 * @return          a iterator of this table
 */
fhash_int_iter fhash_int_iter_new(fhash* table);

/**
 * @brief release a iterator of the fhash table
 *
 * @param iter     a pointer of iterator
 * @return         void
 *
 * @note           user must call this api if the iteration operation is done
 */
void           fhash_int_iter_release(fhash_int_iter* iter);

/**
 * @brief get the next element
 *
 * @param iter     pointer of the iterator
 * @return
 *                 - the next element
 *                 - NULL if reach the end
 */
void*          fhash_int_next(fhash_int_iter* iter);

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
void           fhash_int_foreach(fhash* table, fhash_int_each_cb cb, void* ud);

#ifdef __cplusplus
}
#endif

#endif

