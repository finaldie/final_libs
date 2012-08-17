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
 *  @return a pointer of fcache structure
 */
fcache_t* fcache_create(size_t max_size);

/**
 *  @brief Destroy fcache
 *  @param pcache - which will be destroy
 *  @return void
 */
void      fcache_destroy(fcache_t* pcache);

/**
 *  @brief Add object
 *  @param key
 *  @param value
 *  @param value_size
 *  @param obj_free - if obj_free is not NULL, will call it when update or move
 *                    node
 *  @return 0 - success
 *  @return 1 - failed
 */
int       fcache_set_obj(fcache_t*, const char* key, void* value,
                         size_t value_size, cache_obj_free obj_free);

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
