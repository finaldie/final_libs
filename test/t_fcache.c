#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tu_inc.h"
#include "fcache.h"

void test_fcache()
{
    // couldn't create a 0 size cache
    {
        fcache_t* cache = fcache_create(0, NULL);
        FTU_ASSERT(cache == NULL);
    }

    // create a non-empty cache
    {
        fcache_t* cache = fcache_create(1, NULL);
        FTU_ASSERT(cache != NULL);
        fcache_destroy(cache);
    }

    // no enough space to add a new obj
    {
        fcache_t* cache = fcache_create(1, NULL);
        FTU_ASSERT(cache != NULL);

        char* key = "first";
        char* value = "hello_fcache";
        int ret = fcache_set_obj(cache, key, value, strlen(value) + 1);
        FTU_ASSERT(ret != 0);

        fcache_destroy(cache);
    }

    // add a new obj
    {
        fcache_t* cache = fcache_create(100, NULL);
        FTU_ASSERT(cache != NULL);

        char* key = "first";
        char* value = "hello_fcache";
        int ret = fcache_set_obj(cache, key, value, strlen(value) + 1);
        FTU_ASSERT(ret == 0);
        char* get = fcache_get_obj(cache, key);
        FTU_ASSERT( strcmp(get, value) == 0 );
        FTU_ASSERT( get == value );

        fcache_destroy(cache);
    }

    // add a new obj, and update it
    {
        fcache_t* cache = fcache_create(100, NULL);
        FTU_ASSERT(cache != NULL);

        // add a new obj
        char* key = "first";
        char* value = "hello_fcache";
        int ret = fcache_set_obj(cache, key, value, strlen(value) + 1);
        FTU_ASSERT(ret == 0);
        char* get = fcache_get_obj(cache, key);
        FTU_ASSERT( strcmp(get, value) == 0 );
        FTU_ASSERT( get == value );

        // update it
        char* new_value = "I'm the new value";
        ret = fcache_set_obj(cache, key, new_value, strlen(new_value) + 1);
        FTU_ASSERT(ret == 0);
        char* get_new = fcache_get_obj(cache, key);
        FTU_ASSERT( strcmp(get_new, new_value) == 0 );
        FTU_ASSERT( get_new == new_value );

        fcache_destroy(cache);
    }

    // add 3 new objs, drop the first one
    // this case for two nodes in active list, zero node in inactive list
    // so, it'll balance first, then there is 1 node in active list, 1 node in
    // inactive list(key1), so, drop key1's node from inactive list, then add
    // key3's node into inactive list
    {
        fcache_t* cache = fcache_create(20, NULL);
        FTU_ASSERT(cache != NULL);

        // add a new obj
        char* key1 = "key1";
        char* key2 = "key2";
        char* key3 = "key3";
        char* value1 = "123456789"; // total len = 10
        char* value2 = "234567891"; // total len = 10
        char* value3 = "345678912"; // total len = 10

        // add first
        int ret = fcache_set_obj(cache, key1, value1, strlen(value1) + 1);
        FTU_ASSERT(ret == 0);
        char* get1 = fcache_get_obj(cache, key1);
        FTU_ASSERT( strcmp(get1, value1) == 0 );
        FTU_ASSERT( get1 == value1 );

        // add second
        ret = fcache_set_obj(cache, key2, value2, strlen(value2) + 1);
        FTU_ASSERT(ret == 0);
        char* get2 = fcache_get_obj(cache, key2);
        FTU_ASSERT( strcmp(get2, value2) == 0 );
        FTU_ASSERT( get2 == value2 );

        // add third
        ret = fcache_set_obj(cache, key3, value3, strlen(value3) + 1);
        FTU_ASSERT(ret == 0);
        char* get3 = fcache_get_obj(cache, key3);
        FTU_ASSERT( strcmp(get3, value3) == 0 );
        FTU_ASSERT( get3 == value3 );

        // and the first one will be dropped
        char* get4 = fcache_get_obj(cache, key1);
        char* get5 = fcache_get_obj(cache, key2);
        char* get6 = fcache_get_obj(cache, key3);
        printf("get key1:%s\n", get4);
        printf("get key2:%s\n", get5);
        printf("get key3:%s\n", get6);
        FTU_ASSERT( get4 == NULL );

        fcache_destroy(cache);
    }

    // add 3 new objs, drop the second one
    // this case, there is 1 node in active list(key1), 1 node in inactive list(key2),
    // so, drop key2's node, add key3's node into inactive list
    {
        fcache_t* cache = fcache_create(20, NULL);
        FTU_ASSERT(cache != NULL);

        // add a new obj
        char* key1 = "key1";
        char* key2 = "key2";
        char* key3 = "key3";
        char* value1 = "123456789"; // total len = 10
        char* value2 = "234567891"; // total len = 10
        char* value3 = "345678912"; // total len = 10

        // add first
        int ret = fcache_set_obj(cache, key1, value1, strlen(value1) + 1);
        FTU_ASSERT(ret == 0);
        char* get1 = fcache_get_obj(cache, key1);
        FTU_ASSERT( strcmp(get1, value1) == 0 );
        FTU_ASSERT( get1 == value1 );

        // add second
        ret = fcache_set_obj(cache, key2, value2, strlen(value2) + 1);
        FTU_ASSERT(ret == 0);

        // add third
        ret = fcache_set_obj(cache, key3, value3, strlen(value3) + 1);
        FTU_ASSERT(ret == 0);
        char* get3 = fcache_get_obj(cache, key3);
        FTU_ASSERT( strcmp(get3, value3) == 0 );
        FTU_ASSERT( get3 == value3 );

        // and the first one will be dropped
        char* get4 = fcache_get_obj(cache, key1);
        char* get5 = fcache_get_obj(cache, key2);
        char* get6 = fcache_get_obj(cache, key3);
        printf("get key1:%s\n", get4);
        printf("get key2:%s\n", get5);
        printf("get key3:%s\n", get6);
        FTU_ASSERT( get5 == NULL );

        fcache_destroy(cache);
    }

}
