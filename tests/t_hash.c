#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <fcunit.h>
#include "flibs/fhash.h"
#include "flibs/fhash_int.h"

//=====================FAKE STRUCTURE===========================================
typedef uint64_t data_sz_t;

typedef struct _fhash_node {
    data_sz_t  real_sz;   // size of real memory space

    uint32_t valid:1;
    uint32_t padding:31;  // reserved

    key_sz_t   key_sz;
    value_sz_t value_sz;
    void*      data;

} _fhash_node;

typedef struct {
    size_t used;
    size_t size;
    _fhash_node* node_list;
} _fhash_node_mgr;

typedef struct _fhash {
    uint32_t index_size;
    uint32_t index_used;

    size_t   slots_used;
    _fhash_node_mgr node_mgr[1];
} _fhash;

typedef union {
    struct flags {
        // user flags
        uint32_t auto_rehash:1;
        uint32_t padding:29;    // reserved

        // internal use
        uint32_t rehashing:1;   // doing rehash
        uint32_t performing:1;  // doing delayed actions
    } flags;

    uint32_t value;
} fhash_mask;

struct fhash {
    void*      ud;          // User Data
    uint32_t   iter_refs;   // how many iterators have already refenerced it

    fhash_mask mask;

    fhash_opt  opt;
    _fhash*    current;
    _fhash*    temporary;   // we use it when trigger resizing

    // a delay action list, only *ADD* action can add into it
    _fhash_node_mgr delayed_actions;
};
//=====================FAKE STRUCTURE END=======================================

typedef struct _foreach_data {
    int test_data; // store a number for validation
    int loop_cnt;
    int v1_exist;
    int v2_exist;
    int v3_exist;
    int v4_exist;
    int v5_exist;
    int v6_exist;
} _foreach_data;

// return 0: key1 is same as key2
// return non-zero: key1 is different with key2
int hash_core_compare(const void* key1, key_sz_t key_sz1,
                      const void* key2, key_sz_t key_sz2)
{
    if (key_sz1 != key_sz2) {
        return 1;
    }

    return memcmp(key1, key2, (size_t)key_sz1);
}

int hash_int_each(void* ud, int key, void* value)
{
    _foreach_data* data = (_foreach_data*)ud;
    FCUNIT_ASSERT(data->test_data == 100);
    data->loop_cnt++;

    if (strcmp(value, "test_value") == 0) {
        data->v1_exist = 1;
        FCUNIT_ASSERT(key == 10);
    } else if (strcmp(value, "test_value2") == 0) {
        data->v2_exist = 1;
        FCUNIT_ASSERT(key == 11);
    } else if (strcmp(value, "test_value3") == 0) {
        data->v3_exist = 1;
        FCUNIT_ASSERT(key == 12);
    } else if (strcmp(value, "test_value4") == 0) {
        data->v4_exist = 1;
        FCUNIT_ASSERT(key == 13);
    } else if (strcmp(value, "test_value5") == 0) {
        data->v5_exist = 1;
        FCUNIT_ASSERT(key == 14);
    } else if (strcmp(value, "test_value6") == 0) {
        data->v6_exist = 1;
        FCUNIT_ASSERT(key == 15);
    } else {
        FCUNIT_ASSERT(0);
    }

    return 0;
}

int hash_u64_each(void* ud, uint64_t key, void* value)
{
    _foreach_data* data = (_foreach_data*)ud;
    FCUNIT_ASSERT(data->test_data == 100);
    data->loop_cnt++;

    if (strcmp(value, "test_value") == 0) {
        data->v1_exist = 1;
        FCUNIT_ASSERT(key == 10);
    } else if (strcmp(value, "test_value2") == 0) {
        data->v2_exist = 1;
        FCUNIT_ASSERT(key == 11);
    } else if (strcmp(value, "test_value3") == 0) {
        data->v3_exist = 1;
        FCUNIT_ASSERT(key == 12);
    } else if (strcmp(value, "test_value4") == 0) {
        data->v4_exist = 1;
        FCUNIT_ASSERT(key == 13);
    } else if (strcmp(value, "test_value5") == 0) {
        data->v5_exist = 1;
        FCUNIT_ASSERT(key == 14);
    } else if (strcmp(value, "test_value6") == 0) {
        data->v6_exist = 1;
        FCUNIT_ASSERT(key == 15);
    } else {
        FCUNIT_ASSERT(0);
    }

    return 0;
}

int hash_str_each(void* ud, const char* key, void* value)
{
    _foreach_data* data = (_foreach_data*)ud;
    FCUNIT_ASSERT(data->test_data == 100);
    data->loop_cnt++;

    if (strcmp(value, "test_value") == 0) {
        data->v1_exist = 1;
        FCUNIT_ASSERT(strcmp(key, "test_key") == 0);
    } else if (strcmp(value, "test_value2") == 0) {
        data->v2_exist = 1;
        FCUNIT_ASSERT(strcmp(key, "test_key2") == 0);
    } else if (strcmp(value, "test_value3") == 0) {
        data->v3_exist = 1;
        FCUNIT_ASSERT(strcmp(key, "test_key3") == 0);
    } else if (strcmp(value, "test_value4") == 0) {
        data->v4_exist = 1;
        FCUNIT_ASSERT(strcmp(key, "test_key4") == 0);
    } else if (strcmp(value, "test_value5") == 0) {
        data->v5_exist = 1;
        FCUNIT_ASSERT(strcmp(key, "test_key5") == 0);
    } else if (strcmp(value, "test_value6") == 0) {
        data->v6_exist = 1;
        FCUNIT_ASSERT(strcmp(key, "test_key6") == 0);
    } else {
        FCUNIT_ASSERT(0);
    }

    return 0;
}

void test_hash_int()
{
    // create/delete
    {
        fhash* phash = fhash_int_create(0, FHASH_MASK_NONE);
        fhash_int_delete(phash);
    }

    // set/get/del
    {
        fhash* phash = fhash_int_create(0, FHASH_MASK_NONE);

        int key = 10;
        char value[] = "test_value";
        fhash_int_set(phash, key, value);
        char* ret = fhash_int_get(phash, key);
        FCUNIT_ASSERT(strcmp(ret, value) == 0);
        char* ret1 = fhash_int_del(phash, key);
        FCUNIT_ASSERT(strcmp(ret1, value) == 0);

        fhash_int_delete(phash);
    }

    // iteration
    {
        fhash* phash = fhash_int_create(0, FHASH_MASK_NONE);

        int key = 10;
        char value[] = "test_value";
        int key2 = 11;
        char value2[] = "test_value2";
        int key3 = 12;
        char value3[] = "test_value3";
        int key4 = 13;
        char value4[] = "test_value4";
        int key5 = 14;
        char value5[] = "test_value5";
        int key6 = 15;
        char value6[] = "test_value6";
        int key7 = 16;

        fhash_int_set(phash, key, value);
        fhash_int_set(phash, key2, value2);
        fhash_int_set(phash, key3, value3);
        fhash_int_set(phash, key4, value4);
        fhash_int_set(phash, key5, value5);
        fhash_int_set(phash, key6, value6);
        fhash_int_set(phash, key7, NULL);

        fhash_int_iter iter = fhash_int_iter_new(phash);
        char* data = NULL;
        int v1_exist = 0;
        int v2_exist = 0;
        int v3_exist = 0;
        int v4_exist = 0;
        int v5_exist = 0;
        int v6_exist = 0;
        int v7_exist = 0;
        int loop_cnt = 0;

        while ((data = fhash_int_next(&iter))) {
            loop_cnt++;

            if (strcmp(data, value) == 0) {
                v1_exist = 1;
                FCUNIT_ASSERT(iter.key == 10);
                FCUNIT_ASSERT(strcmp(iter.value, value) == 0);
            } else if (strcmp(data, value2) == 0) {
                v2_exist = 1;
                FCUNIT_ASSERT(iter.key == 11);
                FCUNIT_ASSERT(strcmp(iter.value, value2) == 0);
            } else if (strcmp(data, value3) == 0) {
                v3_exist = 1;
                FCUNIT_ASSERT(iter.key == 12);
                FCUNIT_ASSERT(strcmp(iter.value, value3) == 0);
            } else if (strcmp(data, value4) == 0) {
                v4_exist = 1;
                FCUNIT_ASSERT(iter.key == 13);
                FCUNIT_ASSERT(strcmp(iter.value, value4) == 0);
            } else if (strcmp(data, value5) == 0) {
                v5_exist = 1;
                FCUNIT_ASSERT(iter.key == 14);
                FCUNIT_ASSERT(strcmp(iter.value, value5) == 0);
            } else if (strcmp(data, value6) == 0) {
                v6_exist = 1;
                FCUNIT_ASSERT(iter.key == 15);
                FCUNIT_ASSERT(strcmp(iter.value, value6) == 0);
            } else {
                FCUNIT_ASSERT(0);
            }
        }

        FCUNIT_ASSERT(loop_cnt == 6);
        FCUNIT_ASSERT(v1_exist == 1);
        FCUNIT_ASSERT(v2_exist == 1);
        FCUNIT_ASSERT(v3_exist == 1);
        FCUNIT_ASSERT(v4_exist == 1);
        FCUNIT_ASSERT(v5_exist == 1);
        FCUNIT_ASSERT(v6_exist == 1);
        FCUNIT_ASSERT(v7_exist == 0);

        fhash_int_delete(phash);
    }

    // foreach
    {
        fhash* phash = fhash_int_create(0, FHASH_MASK_NONE);

        int key = 10;
        char value[] = "test_value";
        int key2 = 11;
        char value2[] = "test_value2";
        int key3 = 12;
        char value3[] = "test_value3";
        int key4 = 13;
        char value4[] = "test_value4";
        int key5 = 14;
        char value5[] = "test_value5";
        int key6 = 15;
        char value6[] = "test_value6";

        fhash_int_set(phash, key, value);
        fhash_int_set(phash, key2, value2);
        fhash_int_set(phash, key3, value3);
        fhash_int_set(phash, key4, value4);
        fhash_int_set(phash, key5, value5);
        fhash_int_set(phash, key6, value6);

        _foreach_data ud;
        memset(&ud, 0, sizeof(ud));
        ud.test_data = 100;
        fhash_int_foreach(phash, hash_int_each, &ud);

        FCUNIT_ASSERT(ud.loop_cnt == 6);
        FCUNIT_ASSERT(ud.v1_exist == 1);
        FCUNIT_ASSERT(ud.v2_exist == 1);
        FCUNIT_ASSERT(ud.v3_exist == 1);
        FCUNIT_ASSERT(ud.v4_exist == 1);
        FCUNIT_ASSERT(ud.v5_exist == 1);
        FCUNIT_ASSERT(ud.v6_exist == 1);

        fhash_int_delete(phash);
    }

    // rehash
    {
        fhash* phash = fhash_int_create(0, FHASH_MASK_NONE);

        int key = 10;
        char value[] = "test_value";
        int key2 = 11;
        char value2[] = "test_value2";
        int key3 = 12;
        char value3[] = "test_value3";
        int key4 = 13;
        char value4[] = "test_value4";
        int key5 = 14;
        char value5[] = "test_value5";
        int key6 = 15;
        char value6[] = "test_value6";

        fhash_int_set(phash, key, value);
        fhash_int_set(phash, key2, value2);
        fhash_int_set(phash, key3, value3);
        fhash_int_set(phash, key4, value4);
        fhash_int_set(phash, key5, value5);
        fhash_int_set(phash, key6, value6);

        // before rehash
        {
            _foreach_data ud;
            memset(&ud, 0, sizeof(ud));
            ud.test_data = 100;
            fhash_int_foreach(phash, hash_int_each, &ud);

            FCUNIT_ASSERT(ud.loop_cnt == 6);
            FCUNIT_ASSERT(ud.v1_exist == 1);
            FCUNIT_ASSERT(ud.v2_exist == 1);
            FCUNIT_ASSERT(ud.v3_exist == 1);
            FCUNIT_ASSERT(ud.v4_exist == 1);
            FCUNIT_ASSERT(ud.v5_exist == 1);
            FCUNIT_ASSERT(ud.v6_exist == 1);
        }

        int ret = fhash_rehash(phash, 20);
        FCUNIT_ASSERT(ret == 0);
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->current->index_size == 20);
        FCUNIT_ASSERT(phash->current->index_used == 6);
        FCUNIT_ASSERT(phash->current->slots_used == 6);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);

        // after rehash
        {
            _foreach_data ud;
            memset(&ud, 0, sizeof(ud));
            ud.test_data = 100;
            fhash_int_foreach(phash, hash_int_each, &ud);

            FCUNIT_ASSERT(ud.loop_cnt == 6);
            FCUNIT_ASSERT(ud.v1_exist == 1);
            FCUNIT_ASSERT(ud.v2_exist == 1);
            FCUNIT_ASSERT(ud.v3_exist == 1);
            FCUNIT_ASSERT(ud.v4_exist == 1);
            FCUNIT_ASSERT(ud.v5_exist == 1);
            FCUNIT_ASSERT(ud.v6_exist == 1);
        }

        fhash_int_delete(phash);
    }
}

void test_hash_uint64()
{
    // create/delete
    {
        fhash* phash = fhash_u64_create(0, FHASH_MASK_NONE);
        fhash_u64_delete(phash);
    }

    // set/get/del
    {
        fhash* phash = fhash_u64_create(0, FHASH_MASK_NONE);

        uint64_t key = 10;
        char value[] = "test_value";
        fhash_u64_set(phash, key, value);
        char* ret = fhash_u64_get(phash, key);
        FCUNIT_ASSERT(strcmp(ret, value) == 0);
        char* ret1 = fhash_u64_del(phash, key);
        FCUNIT_ASSERT(strcmp(ret1, value) == 0);

        fhash_u64_delete(phash);
    }

    // iteration
    {
        fhash* phash = fhash_u64_create(0, FHASH_MASK_NONE);

        uint64_t key = 10;
        char value[] = "test_value";
        uint64_t key2 = 11;
        char value2[] = "test_value2";
        uint64_t key3 = 12;
        char value3[] = "test_value3";
        uint64_t key4 = 13;
        char value4[] = "test_value4";
        uint64_t key5 = 14;
        char value5[] = "test_value5";
        uint64_t key6 = 15;
        char value6[] = "test_value6";
        uint64_t key7 = 16;

        fhash_u64_set(phash, key, value);
        fhash_u64_set(phash, key2, value2);
        fhash_u64_set(phash, key3, value3);
        fhash_u64_set(phash, key4, value4);
        fhash_u64_set(phash, key5, value5);
        fhash_u64_set(phash, key6, value6);
        fhash_u64_set(phash, key7, NULL);

        fhash_u64_iter iter = fhash_u64_iter_new(phash);
        char* data = NULL;
        int v1_exist = 0;
        int v2_exist = 0;
        int v3_exist = 0;
        int v4_exist = 0;
        int v5_exist = 0;
        int v6_exist = 0;
        int v7_exist = 0;
        int loop_cnt = 0;

        while ((data = fhash_u64_next(&iter))) {
            loop_cnt++;

            if (strcmp(data, value) == 0) {
                v1_exist = 1;
                FCUNIT_ASSERT(iter.key == 10);
                FCUNIT_ASSERT(strcmp(iter.value, value) == 0);
            } else if (strcmp(data, value2) == 0) {
                v2_exist = 1;
                FCUNIT_ASSERT(iter.key == 11);
                FCUNIT_ASSERT(strcmp(iter.value, value2) == 0);
            } else if (strcmp(data, value3) == 0) {
                v3_exist = 1;
                FCUNIT_ASSERT(iter.key == 12);
                FCUNIT_ASSERT(strcmp(iter.value, value3) == 0);
            } else if (strcmp(data, value4) == 0) {
                v4_exist = 1;
                FCUNIT_ASSERT(iter.key == 13);
                FCUNIT_ASSERT(strcmp(iter.value, value4) == 0);
            } else if (strcmp(data, value5) == 0) {
                v5_exist = 1;
                FCUNIT_ASSERT(iter.key == 14);
                FCUNIT_ASSERT(strcmp(iter.value, value5) == 0);
            } else if (strcmp(data, value6) == 0) {
                v6_exist = 1;
                FCUNIT_ASSERT(iter.key == 15);
                FCUNIT_ASSERT(strcmp(iter.value, value6) == 0);
            } else {
                FCUNIT_ASSERT(0);
            }
        }

        FCUNIT_ASSERT(loop_cnt == 6);
        FCUNIT_ASSERT(v1_exist == 1);
        FCUNIT_ASSERT(v2_exist == 1);
        FCUNIT_ASSERT(v3_exist == 1);
        FCUNIT_ASSERT(v4_exist == 1);
        FCUNIT_ASSERT(v5_exist == 1);
        FCUNIT_ASSERT(v6_exist == 1);
        FCUNIT_ASSERT(v7_exist == 0);

        fhash_u64_delete(phash);
    }

    // foreach
    {
        fhash* phash = fhash_u64_create(0, FHASH_MASK_NONE);

        uint64_t key = 10;
        char value[] = "test_value";
        uint64_t key2 = 11;
        char value2[] = "test_value2";
        uint64_t key3 = 12;
        char value3[] = "test_value3";
        uint64_t key4 = 13;
        char value4[] = "test_value4";
        uint64_t key5 = 14;
        char value5[] = "test_value5";
        uint64_t key6 = 15;
        char value6[] = "test_value6";

        fhash_u64_set(phash, key, value);
        fhash_u64_set(phash, key2, value2);
        fhash_u64_set(phash, key3, value3);
        fhash_u64_set(phash, key4, value4);
        fhash_u64_set(phash, key5, value5);
        fhash_u64_set(phash, key6, value6);

        _foreach_data ud;
        memset(&ud, 0, sizeof(ud));
        ud.test_data = 100;
        fhash_u64_foreach(phash, hash_u64_each, &ud);

        FCUNIT_ASSERT(ud.loop_cnt == 6);
        FCUNIT_ASSERT(ud.v1_exist == 1);
        FCUNIT_ASSERT(ud.v2_exist == 1);
        FCUNIT_ASSERT(ud.v3_exist == 1);
        FCUNIT_ASSERT(ud.v4_exist == 1);
        FCUNIT_ASSERT(ud.v5_exist == 1);
        FCUNIT_ASSERT(ud.v6_exist == 1);

        fhash_u64_delete(phash);
    }

    // rehash
    {
        fhash* phash = fhash_u64_create(0, FHASH_MASK_NONE);

        uint64_t key = 10;
        char value[] = "test_value";
        uint64_t key2 = 11;
        char value2[] = "test_value2";
        uint64_t key3 = 12;
        char value3[] = "test_value3";
        uint64_t key4 = 13;
        char value4[] = "test_value4";
        uint64_t key5 = 14;
        char value5[] = "test_value5";
        uint64_t key6 = 15;
        char value6[] = "test_value6";

        fhash_u64_set(phash, key, value);
        fhash_u64_set(phash, key2, value2);
        fhash_u64_set(phash, key3, value3);
        fhash_u64_set(phash, key4, value4);
        fhash_u64_set(phash, key5, value5);
        fhash_u64_set(phash, key6, value6);

        // before rehash
        {
            _foreach_data ud;
            memset(&ud, 0, sizeof(ud));
            ud.test_data = 100;
            fhash_u64_foreach(phash, hash_u64_each, &ud);

            FCUNIT_ASSERT(ud.loop_cnt == 6);
            FCUNIT_ASSERT(ud.v1_exist == 1);
            FCUNIT_ASSERT(ud.v2_exist == 1);
            FCUNIT_ASSERT(ud.v3_exist == 1);
            FCUNIT_ASSERT(ud.v4_exist == 1);
            FCUNIT_ASSERT(ud.v5_exist == 1);
            FCUNIT_ASSERT(ud.v6_exist == 1);
        }

        int ret = fhash_rehash(phash, 20);
        FCUNIT_ASSERT(ret == 0);
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->current->index_size == 20);
        FCUNIT_ASSERT(phash->current->slots_used == 6);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);

        // after rehash
        {
            _foreach_data ud;
            memset(&ud, 0, sizeof(ud));
            ud.test_data = 100;
            fhash_u64_foreach(phash, hash_u64_each, &ud);

            FCUNIT_ASSERT(ud.loop_cnt == 6);
            FCUNIT_ASSERT(ud.v1_exist == 1);
            FCUNIT_ASSERT(ud.v2_exist == 1);
            FCUNIT_ASSERT(ud.v3_exist == 1);
            FCUNIT_ASSERT(ud.v4_exist == 1);
            FCUNIT_ASSERT(ud.v5_exist == 1);
            FCUNIT_ASSERT(ud.v6_exist == 1);
        }

        fhash_u64_delete(phash);
    }
}

void test_hash_str()
{
    // create/delete
    {
        fhash* phash = fhash_str_create(0, FHASH_MASK_NONE);
        fhash_str_delete(phash);
    }

    // set/get/del
    {
        fhash* phash = fhash_str_create(0, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_str_set(phash, key, value);
        char* ret = fhash_str_get(phash, key);
        FCUNIT_ASSERT(strcmp(ret, value) == 0);
        char* ret1 = fhash_str_del(phash, key);
        FCUNIT_ASSERT(strcmp(ret1, value) == 0);

        fhash_str_delete(phash);
    }

    // iteration
    {
        fhash* phash = fhash_str_create(0, FHASH_MASK_NONE);

        char key[] = "test_key";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char key5[] = "test_key5";
        char key6[] = "test_key6";
        char key7[] = "test_key7";
        char value[] = "test_value";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        char value5[] = "test_value5";
        char value6[] = "test_value6";

        fhash_str_set(phash, key,  value);
        fhash_str_set(phash, key2, value2);
        fhash_str_set(phash, key3, value3);
        fhash_str_set(phash, key4, value4);
        fhash_str_set(phash, key5, value5);
        fhash_str_set(phash, key6, value6);
        fhash_str_set(phash, key7, NULL);
        fhash_str_set(phash, NULL, value6);

        fhash_str_iter iter = fhash_str_iter_new(phash);
        char* data = NULL;
        int v1_exist = 0;
        int v2_exist = 0;
        int v3_exist = 0;
        int v4_exist = 0;
        int v5_exist = 0;
        int v6_exist = 0;
        int v7_exist = 0;
        int v8_exist = 0;
        int loop_cnt = 0;

        while ((data = fhash_str_next(&iter))) {
            loop_cnt++;

            if (strcmp(data, value) == 0) {
                v1_exist = 1;
                FCUNIT_ASSERT(strcmp(iter.key, key) == 0);
                FCUNIT_ASSERT(strcmp(iter.value, value) == 0);
            } else if (strcmp(data, value2) == 0) {
                v2_exist = 1;
                FCUNIT_ASSERT(strcmp(iter.key, key2) == 0);
                FCUNIT_ASSERT(strcmp(iter.value, value2) == 0);
            } else if (strcmp(data, value3) == 0) {
                v3_exist = 1;
                FCUNIT_ASSERT(strcmp(iter.key, key3) == 0);
                FCUNIT_ASSERT(strcmp(iter.value, value3) == 0);
            } else if (strcmp(data, value4) == 0) {
                v4_exist = 1;
                FCUNIT_ASSERT(strcmp(iter.key, key4) == 0);
                FCUNIT_ASSERT(strcmp(iter.value, value4) == 0);
            } else if (strcmp(data, value5) == 0) {
                v5_exist = 1;
                FCUNIT_ASSERT(strcmp(iter.key, key5) == 0);
                FCUNIT_ASSERT(strcmp(iter.value, value5) == 0);
            } else if (strcmp(data, value6) == 0) {
                v6_exist = 1;
                FCUNIT_ASSERT(strcmp(iter.key, key6) == 0);
                FCUNIT_ASSERT(strcmp(iter.value, value6) == 0);
            } else {
                FCUNIT_ASSERT(0);
            }
        }

        FCUNIT_ASSERT(loop_cnt == 6);
        FCUNIT_ASSERT(v1_exist == 1);
        FCUNIT_ASSERT(v2_exist == 1);
        FCUNIT_ASSERT(v3_exist == 1);
        FCUNIT_ASSERT(v4_exist == 1);
        FCUNIT_ASSERT(v5_exist == 1);
        FCUNIT_ASSERT(v6_exist == 1);
        FCUNIT_ASSERT(v7_exist == 0);
        FCUNIT_ASSERT(v8_exist == 0);

        fhash_str_delete(phash);
    }

    // foreach
    {
        fhash* phash = fhash_str_create(0, FHASH_MASK_NONE);
        char key[] = "test_key";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char key5[] = "test_key5";
        char key6[] = "test_key6";
        char value[] = "test_value";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        char value5[] = "test_value5";
        char value6[] = "test_value6";

        fhash_str_set(phash, key, value);
        fhash_str_set(phash, key2, value2);
        fhash_str_set(phash, key3, value3);
        fhash_str_set(phash, key4, value4);
        fhash_str_set(phash, key5, value5);
        fhash_str_set(phash, key6, value6);

        _foreach_data ud;
        memset(&ud, 0, sizeof(ud));
        ud.test_data = 100;
        fhash_str_foreach(phash, hash_str_each, &ud);

        FCUNIT_ASSERT(ud.loop_cnt == 6);
        FCUNIT_ASSERT(ud.v1_exist == 1);
        FCUNIT_ASSERT(ud.v2_exist == 1);
        FCUNIT_ASSERT(ud.v3_exist == 1);
        FCUNIT_ASSERT(ud.v4_exist == 1);
        FCUNIT_ASSERT(ud.v5_exist == 1);
        FCUNIT_ASSERT(ud.v6_exist == 1);

        fhash_str_delete(phash);
    }

    // rehash
    {
        fhash* phash = fhash_str_create(0, FHASH_MASK_NONE);
        char key[] = "test_key";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char key5[] = "test_key5";
        char key6[] = "test_key6";
        char value[] = "test_value";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        char value5[] = "test_value5";
        char value6[] = "test_value6";

        fhash_str_set(phash, key, value);
        fhash_str_set(phash, key2, value2);
        fhash_str_set(phash, key3, value3);
        fhash_str_set(phash, key4, value4);
        fhash_str_set(phash, key5, value5);
        fhash_str_set(phash, key6, value6);

        // before rehash
        {
            _foreach_data ud;
            memset(&ud, 0, sizeof(ud));
            ud.test_data = 100;
            fhash_str_foreach(phash, hash_str_each, &ud);

            FCUNIT_ASSERT(ud.loop_cnt == 6);
            FCUNIT_ASSERT(ud.v1_exist == 1);
            FCUNIT_ASSERT(ud.v2_exist == 1);
            FCUNIT_ASSERT(ud.v3_exist == 1);
            FCUNIT_ASSERT(ud.v4_exist == 1);
            FCUNIT_ASSERT(ud.v5_exist == 1);
            FCUNIT_ASSERT(ud.v6_exist == 1);
        }

        int ret = fhash_rehash(phash, 20);
        FCUNIT_ASSERT(ret == 0);
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->current->index_size == 20);
        FCUNIT_ASSERT(phash->current->slots_used == 6);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);

        // after rehash
        {
            _foreach_data ud;
            memset(&ud, 0, sizeof(ud));
            ud.test_data = 100;
            fhash_str_foreach(phash, hash_str_each, &ud);

            FCUNIT_ASSERT(ud.loop_cnt == 6);
            FCUNIT_ASSERT(ud.v1_exist == 1);
            FCUNIT_ASSERT(ud.v2_exist == 1);
            FCUNIT_ASSERT(ud.v3_exist == 1);
            FCUNIT_ASSERT(ud.v4_exist == 1);
            FCUNIT_ASSERT(ud.v5_exist == 1);
            FCUNIT_ASSERT(ud.v6_exist == 1);
        }

        fhash_str_delete(phash);
    }
}

void test_hash_core()
{
    // test create/delete
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);
        FCUNIT_ASSERT(phash != NULL);
        FCUNIT_ASSERT(phash->ud == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->mask.value == 0);
        FCUNIT_ASSERT(phash->current != NULL);
        FCUNIT_ASSERT(phash->temporary == NULL);

        fhash_delete(phash);
    }

    // test set/get/del, set one key/value in it
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, (key_sz_t)strlen(key),
                  value, (value_sz_t)strlen(value));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, (key_sz_t)strlen(key),
                                           &ret_value_sz);
        FCUNIT_ASSERT(0 == strcmp(ret_value, value));
        FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value));

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        fhash_del(phash, key, (key_sz_t)strlen(key));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 0);
        FCUNIT_ASSERT(phash->current->slots_used == 0);

        fhash_delete(phash);
    }

    // test set/get/fetch_and_del, set one key/value in it
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, (key_sz_t)strlen(key),
                  value, (value_sz_t)strlen(value));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, (key_sz_t)strlen(key),
                                           &ret_value_sz);
        FCUNIT_ASSERT(0 == strcmp(ret_value, value));
        FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value));

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        char data[1024];
        memset(data, 0, 1024);
        char* ret_value1 = fhash_fetch_and_del(phash, key, (key_sz_t)strlen(key),
                                               data, 1024);
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 0);
        FCUNIT_ASSERT(phash->current->slots_used == 0);
        FCUNIT_ASSERT(ret_value1 == data);
        FCUNIT_ASSERT(0 == strcmp(ret_value1, value));
        FCUNIT_ASSERT(0 == strcmp(data, value));

        fhash_delete(phash);
    }

    // test set/get/del, set a group of key/value pairs, to enlarge hash table's
    // node list
    // NOTE: by default, the enlarge coefficient is 2, so we will add 4 items
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        uint32_t index_size = 1;
        fhash* phash = fhash_create(index_size, opt, FHASH_MASK_NONE);

        // set items
        char key1[] = "test_key1";
        char value1[] = "test_value1";
        fhash_set(phash, key1, (key_sz_t)strlen(key1),
                  value1, (value_sz_t)strlen(value1));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 1);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);
        FCUNIT_ASSERT(phash->current->node_mgr[0].size == 1);
        FCUNIT_ASSERT(phash->current->node_mgr[0].used == 1);
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[0].real_sz ==
                   (strlen(key1) + strlen(value1) + 2));
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[0].valid == 1);
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[0].key_sz ==
                   (key_sz_t)strlen(key1));
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[0].value_sz ==
                   (value_sz_t)strlen(value1));

        char key2[] = "test_key2";
        char value2[] = "test_value2";
        fhash_set(phash, key2, (key_sz_t)strlen(key2),
                  value2, (value_sz_t)strlen(value2));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 1);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 2);
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[1].real_sz ==
                   (strlen(key2) + strlen(value2) + 2));
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[1].valid == 1);
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[1].key_sz ==
                   (key_sz_t)strlen(key2));
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[1].value_sz ==
                   (value_sz_t)strlen(value2));

        char key3[] = "test_key3";
        char value3[] = "test_value3";
        fhash_set(phash, key3, (key_sz_t)strlen(key3),
                  value3, (value_sz_t)strlen(value3));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 1);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 3);
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[2].real_sz ==
                   (strlen(key3) + strlen(value3) + 2));
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[2].valid == 1);
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[2].key_sz ==
                   (key_sz_t)strlen(key3));
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[2].value_sz ==
                   (value_sz_t)strlen(value3));

        char key4[] = "test_key4";
        char value4[] = "test_value4";
        fhash_set(phash, key4, (key_sz_t)strlen(key4),
                  value4, (value_sz_t)strlen(value4));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 1);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 4);
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[3].real_sz ==
                   (strlen(key4) + strlen(value4) + 2));
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[3].valid == 1);
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[3].key_sz ==
                   (key_sz_t)strlen(key4));
        FCUNIT_ASSERT(phash->current->node_mgr[0].node_list[3].value_sz ==
                   (value_sz_t)strlen(value4));

        // get items
        {
            value_sz_t ret_value_sz = 0;
            char* ret_value = (char*)fhash_get(phash, key1, (key_sz_t)strlen(key1),
                                               &ret_value_sz);
            FCUNIT_ASSERT(0 == strcmp(ret_value, value1));
            FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value1));
        }

        {
            value_sz_t ret_value_sz = 0;
            char* ret_value = (char*)fhash_get(phash, key2, (key_sz_t)strlen(key2),
                                               &ret_value_sz);
            FCUNIT_ASSERT(0 == strcmp(ret_value, value2));
            FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value2));
        }

        {
            value_sz_t ret_value_sz = 0;
            char* ret_value = (char*)fhash_get(phash, key3, (key_sz_t)strlen(key3),
                                               &ret_value_sz);
            FCUNIT_ASSERT(0 == strcmp(ret_value, value3));
            FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value3));
        }

        {
            value_sz_t ret_value_sz = 0;
            char* ret_value = (char*)fhash_get(phash, key4, (key_sz_t)strlen(key4),
                                               &ret_value_sz);
            FCUNIT_ASSERT(0 == strcmp(ret_value, value4));
            FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value4));
        }

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 1);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 4);

        // delete items
        {
            fhash_del(phash, key1, (key_sz_t)strlen(key1));
            fhash_del(phash, key2, (key_sz_t)strlen(key2));
            fhash_del(phash, key3, (key_sz_t)strlen(key3));
            fhash_del(phash, key4, (key_sz_t)strlen(key4));
            FCUNIT_ASSERT(phash->temporary == NULL);
            FCUNIT_ASSERT(phash->iter_refs == 0);
            FCUNIT_ASSERT(phash->current->index_size == 1);
            FCUNIT_ASSERT(phash->current->index_used == 0);
            FCUNIT_ASSERT(phash->current->slots_used == 0);
        }

        fhash_delete(phash);
    }

    // test iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char value1[] = "test_value1";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        fhash_set(phash, key1, (key_sz_t)strlen(key1), value1, (value_sz_t)strlen(value1));
        fhash_set(phash, key2, (key_sz_t)strlen(key2), value2, (value_sz_t)strlen(value2));
        fhash_set(phash, key3, (key_sz_t)strlen(key3), value3, (value_sz_t)strlen(value3));
        fhash_set(phash, key4, (key_sz_t)strlen(key4), value4, (value_sz_t)strlen(value4));

        fhash_iter iter = fhash_iter_new(phash);
        FCUNIT_ASSERT(phash->iter_refs == 1);

        char* data = NULL;
        int value1_exist = 0;
        int value2_exist = 0;
        int value3_exist = 0;
        int value4_exist = 0;
        int loop_cnt = 0;

        while ((data = (char*)fhash_next(&iter))) {
            loop_cnt++;

            if (strcmp(data, value1) == 0) {
                value1_exist = 1;

                FCUNIT_ASSERT(iter.key_sz == (key_sz_t)strlen(key1));
                FCUNIT_ASSERT(iter.value_sz == (value_sz_t)strlen(value1));
                FCUNIT_ASSERT(0 == strcmp(key1, iter.key));
            } else if (strcmp(data, value2) == 0) {
                value2_exist = 1;

                FCUNIT_ASSERT(iter.key_sz == (key_sz_t)strlen(key2));
                FCUNIT_ASSERT(iter.value_sz == (value_sz_t)strlen(value2));
                FCUNIT_ASSERT(0 == strcmp(key2, iter.key));
            } else if (strcmp(data, value3) == 0) {
                value3_exist = 1;

                FCUNIT_ASSERT(iter.key_sz == (key_sz_t)strlen(key3));
                FCUNIT_ASSERT(iter.value_sz == (value_sz_t)strlen(value3));
                FCUNIT_ASSERT(0 == strcmp(key3, iter.key));
            } else if (strcmp(data, value4) == 0) {
                value4_exist = 1;

                FCUNIT_ASSERT(iter.key_sz == (key_sz_t)strlen(key4));
                FCUNIT_ASSERT(iter.value_sz == (value_sz_t)strlen(value4));
                FCUNIT_ASSERT(0 == strcmp(key4, iter.key));
            }
        }

        FCUNIT_ASSERT(value1_exist == 1);
        FCUNIT_ASSERT(value2_exist == 1);
        FCUNIT_ASSERT(value3_exist == 1);
        FCUNIT_ASSERT(value4_exist == 1);
        FCUNIT_ASSERT(loop_cnt == 4);

        fhash_iter_release(&iter);
        FCUNIT_ASSERT(phash->iter_refs == 0);

        fhash_delete(phash);
    }

    // add key/value pair during iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char value1[] = "test_value1";
        char value2[] = "test_value2";

        fhash_set(phash, key1, (key_sz_t)strlen(key1), value1, (value_sz_t)strlen(value1));
        fhash_set(phash, key2, (key_sz_t)strlen(key2), value2, (value_sz_t)strlen(value2));

        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 2);
        FCUNIT_ASSERT(phash->current->slots_used == 2);

        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char value3[] = "test_value3";
        char value4[] = "test_value4";

        fhash_iter iter = fhash_iter_new(phash);
        char* data = NULL;
        int loop_cnt = 0;
        int key1_exist = 0;
        int key2_exist = 0;

        while ((data = (char*)fhash_next(&iter))) {
            loop_cnt++;

            if (strcmp(data, value1) == 0) {
                key1_exist = 1;

                fhash_set(phash, key3, (key_sz_t)strlen(key3), value3, (value_sz_t)strlen(value3));
                FCUNIT_ASSERT(phash->current->index_size == 10);
                FCUNIT_ASSERT(phash->current->index_used == 2);
                FCUNIT_ASSERT(phash->current->slots_used == 2);

                value_sz_t value_sz = 0;
                char* retv = fhash_get(phash, key3, (key_sz_t)strlen(key3), &value_sz);
                FCUNIT_ASSERT(strcmp(retv, value3) == 0);
                FCUNIT_ASSERT(value_sz == (value_sz_t)strlen(value3));
            } else if (strcmp(data, value2) == 0) {
                key2_exist = 1;

                fhash_set(phash, key4, (key_sz_t)strlen(key4), value4, (value_sz_t)strlen(value4));
                FCUNIT_ASSERT(phash->current->index_size == 10);
                FCUNIT_ASSERT(phash->current->index_used == 2);
                FCUNIT_ASSERT(phash->current->slots_used == 2);

                value_sz_t value_sz = 0;
                char* retv = fhash_get(phash, key4, (key_sz_t)strlen(key4), &value_sz);
                FCUNIT_ASSERT(strcmp(retv, value4) == 0);
                FCUNIT_ASSERT(value_sz == (value_sz_t)strlen(value4));
            } else {
                // shouldn't reach here
                printf("FATAL: data: %s\n", data);
                FCUNIT_ASSERT(0);
            }
        }

        FCUNIT_ASSERT(loop_cnt == 2);
        FCUNIT_ASSERT(key1_exist == 1);
        FCUNIT_ASSERT(key2_exist == 1);
        FCUNIT_ASSERT(phash->delayed_actions.used == 2);
        FCUNIT_ASSERT(phash->delayed_actions.size == 2);

        fhash_iter_release(&iter);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 4);
        FCUNIT_ASSERT(phash->current->slots_used == 4);
        FCUNIT_ASSERT(phash->delayed_actions.size == 2);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);
        fhash_delete(phash);
    }

    // set key during iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char value1[] = "test_value1";
        char value2[] = "test_value2";

        fhash_set(phash, key1, (key_sz_t)strlen(key1), value1, (value_sz_t)strlen(value1));
        fhash_set(phash, key2, (key_sz_t)strlen(key2), value2, (value_sz_t)strlen(value2));

        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 2);
        FCUNIT_ASSERT(phash->current->slots_used == 2);

        char value3[] = "test_value3";
        char value4[] = "test_value4";

        fhash_iter iter = fhash_iter_new(phash);
        char* data = NULL;
        int loop_cnt = 0;
        int key1_exist = 0;
        int key2_exist = 0;

        while ((data = (char*)fhash_next(&iter))) {
            loop_cnt++;

            if (strcmp(data, value1) == 0) {
                key1_exist = 1;

                fhash_set(phash, key1, (key_sz_t)strlen(key1),
                          value3, (value_sz_t)strlen(value3));
                FCUNIT_ASSERT(phash->current->index_size == 10);
                FCUNIT_ASSERT(phash->current->index_used == 2);
                FCUNIT_ASSERT(phash->current->slots_used == 2);
            } else if (strcmp(data, value2) == 0) {
                key2_exist = 1;

                fhash_set(phash, key2, (key_sz_t)strlen(key2),
                          value4, (value_sz_t)strlen(value4));
                FCUNIT_ASSERT(phash->current->index_size == 10);
                FCUNIT_ASSERT(phash->current->index_used == 2);
                FCUNIT_ASSERT(phash->current->slots_used == 2);
            } else {
                // shouldn't reach here
                printf("FATAL: data: %s\n", data);
                FCUNIT_ASSERT(0);
            }
        }

        FCUNIT_ASSERT(loop_cnt == 2);
        FCUNIT_ASSERT(key1_exist == 1);
        FCUNIT_ASSERT(key2_exist == 1);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);

        // check the new values have already be replaced
        value_sz_t new_value1sz = 0;
        char* new_value1 = (char*)fhash_get(phash, key1, (key_sz_t)strlen(key1),
                                            &new_value1sz);
        FCUNIT_ASSERT(0 == strcmp(new_value1, value3));
        FCUNIT_ASSERT((value_sz_t)strlen(new_value1) == new_value1sz);

        value_sz_t new_value2sz = 0;
        char* new_value2 = (char*)fhash_get(phash, key2, (key_sz_t)strlen(key2),
                                            &new_value2sz);
        FCUNIT_ASSERT(0 == strcmp(new_value2, value4));
        FCUNIT_ASSERT((value_sz_t)strlen(new_value2) == new_value2sz);

        fhash_iter_release(&iter);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 2);
        FCUNIT_ASSERT(phash->current->slots_used == 2);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);
        fhash_delete(phash);
    }

    // delete key during iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char value1[] = "test_value1";
        char value2[] = "test_value2";

        fhash_set(phash, key1, (key_sz_t)strlen(key1), value1, (value_sz_t)strlen(value1));
        fhash_set(phash, key2, (key_sz_t)strlen(key2), value2, (value_sz_t)strlen(value2));

        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 2);
        FCUNIT_ASSERT(phash->current->slots_used == 2);

        fhash_iter iter = fhash_iter_new(phash);
        char* data = NULL;
        int loop_cnt = 0;
        int key1_exist = 0;
        int key2_exist = 0;

        while ((data = (char*)fhash_next(&iter))) {
            loop_cnt++;

            if (strcmp(data, value1) == 0) {
                key1_exist = 1;

                fhash_del(phash, key1, (key_sz_t)strlen(key1));
                FCUNIT_ASSERT(phash->current->index_size == 10);
            } else if (strcmp(data, value2) == 0) {
                key2_exist = 1;

                fhash_del(phash, key2, (key_sz_t)strlen(key2));
                FCUNIT_ASSERT(phash->current->index_size == 10);
            } else {
                // shouldn't reach here
                printf("FATAL: data: %s\n", data);
                FCUNIT_ASSERT(0);
            }
        }

        FCUNIT_ASSERT(loop_cnt == 2);
        FCUNIT_ASSERT(key1_exist == 1);
        FCUNIT_ASSERT(key2_exist == 1);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);
        FCUNIT_ASSERT(phash->current->index_used == 0);
        FCUNIT_ASSERT(phash->current->slots_used == 0);

        // check the new values have already be replaced
        value_sz_t new_value1sz = 0;
        char* new_value1 = (char*)fhash_get(phash, key1, (key_sz_t)strlen(key1),
                                            &new_value1sz);
        FCUNIT_ASSERT(new_value1 == NULL);
        FCUNIT_ASSERT(new_value1sz == 0);

        value_sz_t new_value2sz = 0;
        char* new_value2 = (char*)fhash_get(phash, key2, (key_sz_t)strlen(key2),
                                            &new_value2sz);
        FCUNIT_ASSERT(new_value2 == NULL);
        FCUNIT_ASSERT(new_value2sz == 0);

        fhash_iter_release(&iter);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 0);
        FCUNIT_ASSERT(phash->current->slots_used == 0);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);
        fhash_delete(phash);
    }

    // test rehash
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char value1[] = "test_value1";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        fhash_set(phash, key1, (key_sz_t)strlen(key1), value1, (value_sz_t)strlen(value1));
        fhash_set(phash, key2, (key_sz_t)strlen(key2), value2, (value_sz_t)strlen(value2));
        fhash_set(phash, key3, (key_sz_t)strlen(key3), value3, (value_sz_t)strlen(value3));
        fhash_set(phash, key4, (key_sz_t)strlen(key4), value4, (value_sz_t)strlen(value4));

        int ret = fhash_rehash(phash, 20);
        FCUNIT_ASSERT(ret == 0);
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->current->index_size == 20);
        FCUNIT_ASSERT(phash->current->index_used == 4);
        FCUNIT_ASSERT(phash->current->slots_used == 4);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);

        value_sz_t value_sz = 0;
        char* data = fhash_get(phash, key1, (key_sz_t)strlen(key1), &value_sz);
        FCUNIT_ASSERT((size_t)value_sz == strlen(value1));
        FCUNIT_ASSERT(strcmp(data, value1) == 0);

        value_sz = 0;
        data = fhash_get(phash, key2, (key_sz_t)strlen(key2), &value_sz);
        FCUNIT_ASSERT((size_t)value_sz == strlen(value2));
        FCUNIT_ASSERT(strcmp(data, value2) == 0);

        value_sz = 0;
        data = fhash_get(phash, key3, (key_sz_t)strlen(key3), &value_sz);
        FCUNIT_ASSERT((size_t)value_sz == strlen(value3));
        FCUNIT_ASSERT(strcmp(data, value3) == 0);

        value_sz = 0;
        data = fhash_get(phash, key4, (key_sz_t)strlen(key4), &value_sz);
        FCUNIT_ASSERT((size_t)value_sz == strlen(value4));
        FCUNIT_ASSERT(strcmp(data, value4) == 0);

        fhash_delete(phash);
    }

    // test auto rehash
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(1, opt, FHASH_MASK_AUTO_REHASH);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char value1[] = "test_value1";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        fhash_set(phash, key1, (key_sz_t)strlen(key1), value1, (value_sz_t)strlen(value1));
        fhash_set(phash, key2, (key_sz_t)strlen(key2), value2, (value_sz_t)strlen(value2));
        fhash_set(phash, key3, (key_sz_t)strlen(key3), value3, (value_sz_t)strlen(value3));
        fhash_set(phash, key4, (key_sz_t)strlen(key4), value4, (value_sz_t)strlen(value4));

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->current->index_size == 1);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 4);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);

        char key5[] = "test_value5";
        char value5[] = "test_value5";
        fhash_set(phash, key5, (key_sz_t)strlen(key5), value5, (value_sz_t)strlen(value5));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->current->index_size == 2);
        FCUNIT_ASSERT(phash->current->index_used == 2);
        FCUNIT_ASSERT(phash->current->slots_used == 5);
        FCUNIT_ASSERT(phash->delayed_actions.size == 0);
        FCUNIT_ASSERT(phash->delayed_actions.used == 0);
        FCUNIT_ASSERT(phash->mask.flags.rehashing == 0);
        FCUNIT_ASSERT(phash->mask.flags.performing == 0);
        FCUNIT_ASSERT(phash->mask.flags.auto_rehash == 1);

        value_sz_t value_sz = 0;
        char* data = fhash_get(phash, key1, (key_sz_t)strlen(key1), &value_sz);
        FCUNIT_ASSERT((size_t)value_sz == strlen(value1));
        FCUNIT_ASSERT(strcmp(data, value1) == 0);

        value_sz = 0;
        data = fhash_get(phash, key2, (key_sz_t)strlen(key2), &value_sz);
        FCUNIT_ASSERT((size_t)value_sz == strlen(value2));
        FCUNIT_ASSERT(strcmp(data, value2) == 0);

        value_sz = 0;
        data = fhash_get(phash, key3, (key_sz_t)strlen(key3), &value_sz);
        FCUNIT_ASSERT((size_t)value_sz == strlen(value3));
        FCUNIT_ASSERT(strcmp(data, value3) == 0);

        value_sz = 0;
        data = fhash_get(phash, key4, (key_sz_t)strlen(key4), &value_sz);
        FCUNIT_ASSERT((size_t)value_sz == strlen(value4));
        FCUNIT_ASSERT(strcmp(data, value4) == 0);

        value_sz = 0;
        data = fhash_get(phash, key5, (key_sz_t)strlen(key5), &value_sz);
        FCUNIT_ASSERT((size_t)value_sz == strlen(value5));
        FCUNIT_ASSERT(strcmp(data, value5) == 0);

        fhash_delete(phash);
    }

    // rehash duiring iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char value1[] = "test_value1";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        fhash_set(phash, key1, (key_sz_t)strlen(key1), value1, (value_sz_t)strlen(value1));
        fhash_set(phash, key2, (key_sz_t)strlen(key2), value2, (value_sz_t)strlen(value2));
        fhash_set(phash, key3, (key_sz_t)strlen(key3), value3, (value_sz_t)strlen(value3));
        fhash_set(phash, key4, (key_sz_t)strlen(key4), value4, (value_sz_t)strlen(value4));

        fhash_iter iter = fhash_iter_new(phash);
        FCUNIT_ASSERT(phash->iter_refs == 1);

        char* data = NULL;
        int value1_exist = 0;
        int value2_exist = 0;
        int value3_exist = 0;
        int value4_exist = 0;
        int loop_cnt = 0;

        while ((data = (char*)fhash_next(&iter))) {
            loop_cnt++;

            if (strcmp(data, value1) == 0) {
                value1_exist = 1;

                FCUNIT_ASSERT(iter.key_sz == (key_sz_t)strlen(key1));
                FCUNIT_ASSERT(iter.value_sz == (value_sz_t)strlen(value1));
                FCUNIT_ASSERT(0 == strcmp(key1, iter.key));

                int ret = fhash_rehash(phash, 20);
                FCUNIT_ASSERT(ret == 1);
            } else if (strcmp(data, value2) == 0) {
                value2_exist = 1;

                FCUNIT_ASSERT(iter.key_sz == (key_sz_t)strlen(key2));
                FCUNIT_ASSERT(iter.value_sz == (value_sz_t)strlen(value2));
                FCUNIT_ASSERT(0 == strcmp(key2, iter.key));

                int ret = fhash_rehash(phash, 20);
                FCUNIT_ASSERT(ret == 1);
            } else if (strcmp(data, value3) == 0) {
                value3_exist = 1;

                FCUNIT_ASSERT(iter.key_sz == (key_sz_t)strlen(key3));
                FCUNIT_ASSERT(iter.value_sz == (value_sz_t)strlen(value3));
                FCUNIT_ASSERT(0 == strcmp(key3, iter.key));

                int ret = fhash_rehash(phash, 20);
                FCUNIT_ASSERT(ret == 1);
            } else if (strcmp(data, value4) == 0) {
                value4_exist = 1;

                FCUNIT_ASSERT(iter.key_sz == (key_sz_t)strlen(key4));
                FCUNIT_ASSERT(iter.value_sz == (value_sz_t)strlen(value4));
                FCUNIT_ASSERT(0 == strcmp(key4, iter.key));

                int ret = fhash_rehash(phash, 20);
                FCUNIT_ASSERT(ret == 1);
            }
        }

        FCUNIT_ASSERT(value1_exist == 1);
        FCUNIT_ASSERT(value2_exist == 1);
        FCUNIT_ASSERT(value3_exist == 1);
        FCUNIT_ASSERT(value4_exist == 1);
        FCUNIT_ASSERT(loop_cnt == 4);

        fhash_iter_release(&iter);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 4);
        FCUNIT_ASSERT(phash->current->slots_used == 4);
        FCUNIT_ASSERT(phash->mask.flags.rehashing == 0);
        FCUNIT_ASSERT(phash->mask.flags.performing == 0);

        fhash_delete(phash);
    }

    // test reset the value of a key
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, (key_sz_t)strlen(key), value, (value_sz_t)strlen(value));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, (key_sz_t)strlen(key),
                                           &ret_value_sz);
        FCUNIT_ASSERT(0 == strcmp(ret_value, value));
        FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value));

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        char value2[] = "test_value2";
        fhash_set(phash, key, (key_sz_t)strlen(key), value2, (value_sz_t)strlen(value2));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz2 = 0;
        char* ret_value2 = (char*)fhash_get(phash, key, (key_sz_t)strlen(key),
                                           &ret_value_sz2);
        FCUNIT_ASSERT(0 == strcmp(ret_value2, value2));
        FCUNIT_ASSERT((size_t)ret_value_sz2 == strlen(value2));

        fhash_delete(phash);
    }

    // test set a empty key or empty value
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key[] = "";
        char value[] = "test_value";
        fhash_set(phash, key, (key_sz_t)strlen(key), value, (value_sz_t)strlen(value));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 0);
        FCUNIT_ASSERT(phash->current->slots_used == 0);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, (key_sz_t)strlen(key),
                                           &ret_value_sz);
        FCUNIT_ASSERT(ret_value == NULL);
        FCUNIT_ASSERT(ret_value_sz == 0);

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 0);
        FCUNIT_ASSERT(phash->current->slots_used == 0);

        char key2[] = "test_key2";
        char value2[] = "";
        fhash_set(phash, key2, (key_sz_t)strlen(key2), value2, (value_sz_t)strlen(value2));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 0);
        FCUNIT_ASSERT(phash->current->slots_used == 0);

        value_sz_t ret_value_sz2 = 0;
        char* ret_value2 = (char*)fhash_get(phash, key, (key_sz_t)strlen(key),
                                           &ret_value_sz2);
        FCUNIT_ASSERT(ret_value2 == NULL);
        FCUNIT_ASSERT(ret_value_sz2 == 0);

        fhash_delete(phash);
    }

    // test delete a empty key
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, (key_sz_t)strlen(key), value, (value_sz_t)strlen(value));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, (key_sz_t)strlen(key),
                                           &ret_value_sz);
        FCUNIT_ASSERT(strcmp(ret_value, value) == 0);
        FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value));

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        char key2[] = "";
        fhash_del(phash, key2, (key_sz_t)strlen(key2));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        fhash_delete(phash);
    }

    // test delete a non-exist key
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, (key_sz_t)strlen(key), value, (value_sz_t)strlen(value));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, (key_sz_t)strlen(key),
                                           &ret_value_sz);
        FCUNIT_ASSERT(strcmp(ret_value, value) == 0);
        FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value));

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        char key2[] = "test_key2";
        fhash_del(phash, key2, (key_sz_t)strlen(key2));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        fhash_delete(phash);
    }

    // test get a non-exist key
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, (key_sz_t)strlen(key), value, (value_sz_t)strlen(value));
        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, (key_sz_t)strlen(key),
                                           &ret_value_sz);
        FCUNIT_ASSERT(strcmp(ret_value, value) == 0);
        FCUNIT_ASSERT((size_t)ret_value_sz == strlen(value));

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        char key2[] = "test_key2";
        value_sz_t new_valuesz = 0;
        char* data = (char*)fhash_get(phash, key2, (key_sz_t)strlen(key2), &new_valuesz);
        FCUNIT_ASSERT(new_valuesz == 0);
        FCUNIT_ASSERT(data == NULL);

        FCUNIT_ASSERT(phash->temporary == NULL);
        FCUNIT_ASSERT(phash->iter_refs == 0);
        FCUNIT_ASSERT(phash->current->index_size == 10);
        FCUNIT_ASSERT(phash->current->index_used == 1);
        FCUNIT_ASSERT(phash->current->slots_used == 1);

        fhash_delete(phash);
    }
}

int main(int argc, char** argv)
{
    FCUNIT_RUN(test_hash_core);
    FCUNIT_RUN(test_hash_int);
    FCUNIT_RUN(test_hash_str);
    FCUNIT_RUN(test_hash_uint64);

    return 0;
}
