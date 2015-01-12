/*
 * =====================================================================================
 *
 *       Filename:  fcache.h
 *
 *    Description:  A Fast cache with LRU
 *
 *        Version:  1.0
 *        Created:  08/16/2012 15:18:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *
 * =====================================================================================
 */

#ifndef _FLIBS_FCACHE_H_
#define _FLIBS_FCACHE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fcache fcache_t;
typedef void (*cache_obj_free)(void* data);

/**
 *  @brief Create fcache
 *  @param max_size - max size of cache
 *  @param obj_free - if obj_free is not NULL, will call it when the node is not
 *                    in the cache
 *  @return a pointer of fcache structure
 */
fcache_t* fcache_create(size_t max_size, cache_obj_free obj_free);

/**
 *  @brief Destroy fcache
 *  @param pcache - which will be destroy
 *  @return void
 */
void      fcache_destroy(fcache_t* pcache);

/**
 *  @brief Add object, it has three behavior:
 *         1. if key not in the cache, fcache will add it into cache
 *         2. if key in the cache, fcache will update it
 *         3. if key in the cache, but user_data arg is NULL, fcache will drop
 *            this node
 *  @param key - the key of user_data
 *  @param key_size - size of key
 *  @param user_data - user need to construct it by himself
 *  @param data_size - size of user_data
 *  @return 0 - success
 *  @return 1 - failed
 */
int       fcache_set_obj(fcache_t*, const char* key, size_t key_size,
                         void* user_data, size_t data_size);

/**
 *  @brief Get object
 *  @param key
 *  @return the point of object
 *  @return NULL - failed
 */
void*     fcache_get_obj(fcache_t*, const char* key);

#ifdef __cplusplus
}
#endif

#endif
