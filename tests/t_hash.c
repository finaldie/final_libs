//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "ftu/ftu_inc.h"
#include "fhash/fhash.h"
#include "inc.h"

#define LOOP 10000

static int total_count = 0;

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
    _fhash_node_mgr node_mgr[0];
} _fhash;

typedef union {
    struct {
        // user flags
        uint32_t auto_rehash:1;
        uint32_t padding:29;    // reserved

        // internal use
        uint32_t rehashing:1;   // doing rehash
        uint32_t performing:1;  // doing delayed actions
    };

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

int test_print(void* data __attribute__((unused)))
{
    //printf("key=%s(%d), value=%s\n", key, hash_atoi(key), (char*)data);
    total_count++;
    return 0;
}

// return 0: key1 is same as key2
// return non-zero: key1 is different with key2
int hash_core_compare(const void* key1, key_sz_t key_sz1,
                      const void* key2, key_sz_t key_sz2)
{
    if (key_sz1 != key_sz2) {
        return 1;
    }

    return memcmp(key1, key2, key_sz1);
}

void test_hash_int()
{

}

void test_hash_uint64()
{

}

void test_hash_str()
{

}

void test_hash_core()
{
    // test create/delete
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);
        FTU_ASSERT(phash != NULL);
        FTU_ASSERT(phash->ud == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->mask.value == 0);
        FTU_ASSERT(phash->current != NULL);
        FTU_ASSERT(phash->temporary == NULL);

        fhash_delete(phash);
    }

    // test set/get/del, set one key/value in it
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, strlen(key), value, strlen(value));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, strlen(key),
                                           &ret_value_sz);
        FTU_ASSERT(0 == strcmp(ret_value, value));
        FTU_ASSERT((size_t)ret_value_sz == strlen(value));

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        fhash_del(phash, key, strlen(key));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 0);
        FTU_ASSERT(phash->current->slots_used == 0);

        fhash_delete(phash);
    }

    // test set/get/fetch_and_del, set one key/value in it
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, strlen(key), value, strlen(value));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, strlen(key),
                                           &ret_value_sz);
        FTU_ASSERT(0 == strcmp(ret_value, value));
        FTU_ASSERT((size_t)ret_value_sz == strlen(value));

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        char data[1024];
        memset(data, 0, 1024);
        char* ret_value1 = fhash_fetch_and_del(phash, key, strlen(key),
                                               data, 1024);
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 0);
        FTU_ASSERT(phash->current->slots_used == 0);
        FTU_ASSERT(ret_value1 == data);
        FTU_ASSERT(0 == strcmp(ret_value1, value));
        FTU_ASSERT(0 == strcmp(data, value));

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
        fhash* phash = fhash_create(index_size, opt, NULL, FHASH_MASK_NONE);

        // set items
        char key1[] = "test_key1";
        char value1[] = "test_value1";
        fhash_set(phash, key1, strlen(key1), value1, strlen(value1));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 1);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);
        FTU_ASSERT(phash->current->node_mgr[0].size == 1);
        FTU_ASSERT(phash->current->node_mgr[0].used == 1);
        FTU_ASSERT(phash->current->node_mgr[0].node_list[0].real_sz ==
                   (strlen(key1) + strlen(value1) + 2));
        FTU_ASSERT(phash->current->node_mgr[0].node_list[0].valid == 1);
        FTU_ASSERT(phash->current->node_mgr[0].node_list[0].key_sz ==
                   (key_sz_t)strlen(key1));
        FTU_ASSERT(phash->current->node_mgr[0].node_list[0].value_sz ==
                   (value_sz_t)strlen(value1));

        char key2[] = "test_key2";
        char value2[] = "test_value2";
        fhash_set(phash, key2, strlen(key2), value2, strlen(value2));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 1);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 2);
        FTU_ASSERT(phash->current->node_mgr[0].node_list[1].real_sz ==
                   (strlen(key2) + strlen(value2) + 2));
        FTU_ASSERT(phash->current->node_mgr[0].node_list[1].valid == 1);
        FTU_ASSERT(phash->current->node_mgr[0].node_list[1].key_sz ==
                   (key_sz_t)strlen(key2));
        FTU_ASSERT(phash->current->node_mgr[0].node_list[1].value_sz ==
                   (value_sz_t)strlen(value2));

        char key3[] = "test_key3";
        char value3[] = "test_value3";
        fhash_set(phash, key3, strlen(key3), value3, strlen(value3));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 1);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 3);
        FTU_ASSERT(phash->current->node_mgr[0].node_list[2].real_sz ==
                   (strlen(key3) + strlen(value3) + 2));
        FTU_ASSERT(phash->current->node_mgr[0].node_list[2].valid == 1);
        FTU_ASSERT(phash->current->node_mgr[0].node_list[2].key_sz ==
                   (key_sz_t)strlen(key3));
        FTU_ASSERT(phash->current->node_mgr[0].node_list[2].value_sz ==
                   (value_sz_t)strlen(value3));

        char key4[] = "test_key4";
        char value4[] = "test_value4";
        fhash_set(phash, key4, strlen(key4), value4, strlen(value4));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 1);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 4);
        FTU_ASSERT(phash->current->node_mgr[0].node_list[3].real_sz ==
                   (strlen(key4) + strlen(value4) + 2));
        FTU_ASSERT(phash->current->node_mgr[0].node_list[3].valid == 1);
        FTU_ASSERT(phash->current->node_mgr[0].node_list[3].key_sz ==
                   (key_sz_t)strlen(key4));
        FTU_ASSERT(phash->current->node_mgr[0].node_list[3].value_sz ==
                   (value_sz_t)strlen(value4));

        // get items
        {
            value_sz_t ret_value_sz = 0;
            char* ret_value = (char*)fhash_get(phash, key1, strlen(key1),
                                               &ret_value_sz);
            FTU_ASSERT(0 == strcmp(ret_value, value1));
            FTU_ASSERT((size_t)ret_value_sz == strlen(value1));
        }

        {
            value_sz_t ret_value_sz = 0;
            char* ret_value = (char*)fhash_get(phash, key2, strlen(key2),
                                               &ret_value_sz);
            FTU_ASSERT(0 == strcmp(ret_value, value2));
            FTU_ASSERT((size_t)ret_value_sz == strlen(value2));
        }

        {
            value_sz_t ret_value_sz = 0;
            char* ret_value = (char*)fhash_get(phash, key3, strlen(key3),
                                               &ret_value_sz);
            FTU_ASSERT(0 == strcmp(ret_value, value3));
            FTU_ASSERT((size_t)ret_value_sz == strlen(value3));
        }

        {
            value_sz_t ret_value_sz = 0;
            char* ret_value = (char*)fhash_get(phash, key4, strlen(key4),
                                               &ret_value_sz);
            FTU_ASSERT(0 == strcmp(ret_value, value4));
            FTU_ASSERT((size_t)ret_value_sz == strlen(value4));
        }

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 1);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 4);

        // delete items
        {
            fhash_del(phash, key1, strlen(key1));
            fhash_del(phash, key2, strlen(key2));
            fhash_del(phash, key3, strlen(key3));
            fhash_del(phash, key4, strlen(key4));
            FTU_ASSERT(phash->temporary == NULL);
            FTU_ASSERT(phash->iter_refs == 0);
            FTU_ASSERT(phash->current->index_size == 1);
            FTU_ASSERT(phash->current->index_used == 0);
            FTU_ASSERT(phash->current->slots_used == 0);
        }

        fhash_delete(phash);
    }

    // test iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char value1[] = "test_value1";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        fhash_set(phash, key1, strlen(key1), value1, strlen(value1));
        fhash_set(phash, key2, strlen(key2), value2, strlen(value2));
        fhash_set(phash, key3, strlen(key3), value3, strlen(value3));
        fhash_set(phash, key4, strlen(key4), value4, strlen(value4));

        fhash_iter iter = fhash_iter_new(phash);
        FTU_ASSERT(phash->iter_refs == 1);

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

                FTU_ASSERT(iter.key_sz == (key_sz_t)strlen(key1));
                FTU_ASSERT(iter.value_sz == (value_sz_t)strlen(value1));
                FTU_ASSERT(0 == strcmp(key1, iter.key));
            } else if (strcmp(data, value2) == 0) {
                value2_exist = 1;

                FTU_ASSERT(iter.key_sz == (key_sz_t)strlen(key2));
                FTU_ASSERT(iter.value_sz == (value_sz_t)strlen(value2));
                FTU_ASSERT(0 == strcmp(key2, iter.key));
            } else if (strcmp(data, value3) == 0) {
                value3_exist = 1;

                FTU_ASSERT(iter.key_sz == (key_sz_t)strlen(key3));
                FTU_ASSERT(iter.value_sz == (value_sz_t)strlen(value3));
                FTU_ASSERT(0 == strcmp(key3, iter.key));
            } else if (strcmp(data, value4) == 0) {
                value4_exist = 1;

                FTU_ASSERT(iter.key_sz == (key_sz_t)strlen(key4));
                FTU_ASSERT(iter.value_sz == (value_sz_t)strlen(value4));
                FTU_ASSERT(0 == strcmp(key4, iter.key));
            }
        }

        FTU_ASSERT(value1_exist == 1);
        FTU_ASSERT(value2_exist == 1);
        FTU_ASSERT(value3_exist == 1);
        FTU_ASSERT(value4_exist == 1);
        FTU_ASSERT(loop_cnt == 4);

        fhash_iter_release(&iter);
        FTU_ASSERT(phash->iter_refs == 0);

        fhash_delete(phash);
    }

    // add key/value pair during iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char value1[] = "test_value1";
        char value2[] = "test_value2";

        fhash_set(phash, key1, strlen(key1), value1, strlen(value1));
        fhash_set(phash, key2, strlen(key2), value2, strlen(value2));

        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 2);
        FTU_ASSERT(phash->current->slots_used == 2);

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

                fhash_set(phash, key3, strlen(key3), value3, strlen(value3));
                FTU_ASSERT(phash->current->index_size == 10);
                FTU_ASSERT(phash->current->index_used == 2);
                FTU_ASSERT(phash->current->slots_used == 2);
            } else if (strcmp(data, value2) == 0) {
                key2_exist = 1;

                fhash_set(phash, key4, strlen(key4), value4, strlen(value4));
                FTU_ASSERT(phash->current->index_size == 10);
                FTU_ASSERT(phash->current->index_used == 2);
                FTU_ASSERT(phash->current->slots_used == 2);
            } else {
                // shouldn't reach here
                printf("FATAL: data: %s\n", data);
                FTU_ASSERT(0);
            }
        }

        FTU_ASSERT(loop_cnt == 2);
        FTU_ASSERT(key1_exist == 1);
        FTU_ASSERT(key2_exist == 1);
        FTU_ASSERT(phash->delayed_actions.used == 2);
        FTU_ASSERT(phash->delayed_actions.size == 2);

        fhash_iter_release(&iter);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 4);
        FTU_ASSERT(phash->current->slots_used == 4);
        FTU_ASSERT(phash->delayed_actions.size == 2);
        FTU_ASSERT(phash->delayed_actions.used == 0);
        fhash_delete(phash);
    }

    // set key during iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char value1[] = "test_value1";
        char value2[] = "test_value2";

        fhash_set(phash, key1, strlen(key1), value1, strlen(value1));
        fhash_set(phash, key2, strlen(key2), value2, strlen(value2));

        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 2);
        FTU_ASSERT(phash->current->slots_used == 2);

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

                fhash_set(phash, key1, strlen(key1), value3, strlen(value3));
                FTU_ASSERT(phash->current->index_size == 10);
                FTU_ASSERT(phash->current->index_used == 2);
                FTU_ASSERT(phash->current->slots_used == 2);
            } else if (strcmp(data, value2) == 0) {
                key2_exist = 1;

                fhash_set(phash, key2, strlen(key2), value4, strlen(value4));
                FTU_ASSERT(phash->current->index_size == 10);
                FTU_ASSERT(phash->current->index_used == 2);
                FTU_ASSERT(phash->current->slots_used == 2);
            } else {
                // shouldn't reach here
                printf("FATAL: data: %s\n", data);
                FTU_ASSERT(0);
            }
        }

        FTU_ASSERT(loop_cnt == 2);
        FTU_ASSERT(key1_exist == 1);
        FTU_ASSERT(key2_exist == 1);
        FTU_ASSERT(phash->delayed_actions.used == 0);
        FTU_ASSERT(phash->delayed_actions.size == 0);

        // check the new values have already be replaced
        value_sz_t new_value1sz = 0;
        char* new_value1 = (char*)fhash_get(phash, key1, strlen(key1),
                                            &new_value1sz);
        FTU_ASSERT(0 == strcmp(new_value1, value3));
        FTU_ASSERT((value_sz_t)strlen(new_value1) == new_value1sz);

        value_sz_t new_value2sz = 0;
        char* new_value2 = (char*)fhash_get(phash, key2, strlen(key2),
                                            &new_value2sz);
        FTU_ASSERT(0 == strcmp(new_value2, value4));
        FTU_ASSERT((value_sz_t)strlen(new_value2) == new_value2sz);

        fhash_iter_release(&iter);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 2);
        FTU_ASSERT(phash->current->slots_used == 2);
        FTU_ASSERT(phash->delayed_actions.size == 0);
        FTU_ASSERT(phash->delayed_actions.used == 0);
        fhash_delete(phash);
    }

    // delete key during iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char value1[] = "test_value1";
        char value2[] = "test_value2";

        fhash_set(phash, key1, strlen(key1), value1, strlen(value1));
        fhash_set(phash, key2, strlen(key2), value2, strlen(value2));

        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 2);
        FTU_ASSERT(phash->current->slots_used == 2);

        fhash_iter iter = fhash_iter_new(phash);
        char* data = NULL;
        int loop_cnt = 0;
        int key1_exist = 0;
        int key2_exist = 0;

        while ((data = (char*)fhash_next(&iter))) {
            loop_cnt++;

            if (strcmp(data, value1) == 0) {
                key1_exist = 1;

                fhash_del(phash, key1, strlen(key1));
                FTU_ASSERT(phash->current->index_size == 10);
            } else if (strcmp(data, value2) == 0) {
                key2_exist = 1;

                fhash_del(phash, key2, strlen(key2));
                FTU_ASSERT(phash->current->index_size == 10);
            } else {
                // shouldn't reach here
                printf("FATAL: data: %s\n", data);
                FTU_ASSERT(0);
            }
        }

        FTU_ASSERT(loop_cnt == 2);
        FTU_ASSERT(key1_exist == 1);
        FTU_ASSERT(key2_exist == 1);
        FTU_ASSERT(phash->delayed_actions.used == 0);
        FTU_ASSERT(phash->delayed_actions.size == 0);
        FTU_ASSERT(phash->current->index_used == 0);
        FTU_ASSERT(phash->current->slots_used == 0);

        // check the new values have already be replaced
        value_sz_t new_value1sz = 0;
        char* new_value1 = (char*)fhash_get(phash, key1, strlen(key1),
                                            &new_value1sz);
        FTU_ASSERT(new_value1 == NULL);
        FTU_ASSERT(new_value1sz == 0);

        value_sz_t new_value2sz = 0;
        char* new_value2 = (char*)fhash_get(phash, key2, strlen(key2),
                                            &new_value2sz);
        FTU_ASSERT(new_value2 == NULL);
        FTU_ASSERT(new_value2sz == 0);

        fhash_iter_release(&iter);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 0);
        FTU_ASSERT(phash->current->slots_used == 0);
        FTU_ASSERT(phash->delayed_actions.size == 0);
        FTU_ASSERT(phash->delayed_actions.used == 0);
        fhash_delete(phash);
    }

    // test rehash
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char value1[] = "test_value1";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        fhash_set(phash, key1, strlen(key1), value1, strlen(value1));
        fhash_set(phash, key2, strlen(key2), value2, strlen(value2));
        fhash_set(phash, key3, strlen(key3), value3, strlen(value3));
        fhash_set(phash, key4, strlen(key4), value4, strlen(value4));

        int ret = fhash_rehash(phash, 20);
        FTU_ASSERT(ret == 0);
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->current->index_size == 20);
        FTU_ASSERT(phash->current->index_used == 4);
        FTU_ASSERT(phash->current->slots_used == 4);
        FTU_ASSERT(phash->delayed_actions.size == 0);
        FTU_ASSERT(phash->delayed_actions.used == 0);

        value_sz_t value_sz = 0;
        char* data = fhash_get(phash, key1, strlen(key1), &value_sz);
        FTU_ASSERT((size_t)value_sz == strlen(value1));
        FTU_ASSERT(strcmp(data, value1) == 0);

        value_sz = 0;
        data = fhash_get(phash, key2, strlen(key2), &value_sz);
        FTU_ASSERT((size_t)value_sz == strlen(value2));
        FTU_ASSERT(strcmp(data, value2) == 0);

        value_sz = 0;
        data = fhash_get(phash, key3, strlen(key3), &value_sz);
        FTU_ASSERT((size_t)value_sz == strlen(value3));
        FTU_ASSERT(strcmp(data, value3) == 0);

        value_sz = 0;
        data = fhash_get(phash, key4, strlen(key4), &value_sz);
        FTU_ASSERT((size_t)value_sz == strlen(value4));
        FTU_ASSERT(strcmp(data, value4) == 0);

        fhash_delete(phash);
    }

    // test auto rehash
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(1, opt, NULL, FHASH_MASK_AUTO_REHASH);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char value1[] = "test_value1";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        fhash_set(phash, key1, strlen(key1), value1, strlen(value1));
        fhash_set(phash, key2, strlen(key2), value2, strlen(value2));
        fhash_set(phash, key3, strlen(key3), value3, strlen(value3));
        fhash_set(phash, key4, strlen(key4), value4, strlen(value4));

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->current->index_size == 1);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 4);
        FTU_ASSERT(phash->delayed_actions.size == 0);
        FTU_ASSERT(phash->delayed_actions.used == 0);

        printf("=============================\n");
        char key5[] = "test_value5";
        char value5[] = "test_value5";
        fhash_set(phash, key5, strlen(key5), value5, strlen(value5));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->current->index_size == 2);
        FTU_ASSERT(phash->current->index_used == 2);
        FTU_ASSERT(phash->current->slots_used == 5);
        FTU_ASSERT(phash->delayed_actions.size == 0);
        FTU_ASSERT(phash->delayed_actions.used == 0);

        value_sz_t value_sz = 0;
        char* data = fhash_get(phash, key1, strlen(key1), &value_sz);
        FTU_ASSERT((size_t)value_sz == strlen(value1));
        FTU_ASSERT(strcmp(data, value1) == 0);

        value_sz = 0;
        data = fhash_get(phash, key2, strlen(key2), &value_sz);
        FTU_ASSERT((size_t)value_sz == strlen(value2));
        FTU_ASSERT(strcmp(data, value2) == 0);

        value_sz = 0;
        data = fhash_get(phash, key3, strlen(key3), &value_sz);
        FTU_ASSERT((size_t)value_sz == strlen(value3));
        FTU_ASSERT(strcmp(data, value3) == 0);

        value_sz = 0;
        data = fhash_get(phash, key4, strlen(key4), &value_sz);
        FTU_ASSERT((size_t)value_sz == strlen(value4));
        FTU_ASSERT(strcmp(data, value4) == 0);

        value_sz = 0;
        data = fhash_get(phash, key5, strlen(key5), &value_sz);
        FTU_ASSERT((size_t)value_sz == strlen(value5));
        FTU_ASSERT(strcmp(data, value5) == 0);

        fhash_delete(phash);
    }

    // rehash duiring iteration
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key1[] = "test_key1";
        char key2[] = "test_key2";
        char key3[] = "test_key3";
        char key4[] = "test_key4";
        char value1[] = "test_value1";
        char value2[] = "test_value2";
        char value3[] = "test_value3";
        char value4[] = "test_value4";
        fhash_set(phash, key1, strlen(key1), value1, strlen(value1));
        fhash_set(phash, key2, strlen(key2), value2, strlen(value2));
        fhash_set(phash, key3, strlen(key3), value3, strlen(value3));
        fhash_set(phash, key4, strlen(key4), value4, strlen(value4));

        fhash_iter iter = fhash_iter_new(phash);
        FTU_ASSERT(phash->iter_refs == 1);

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

                FTU_ASSERT(iter.key_sz == (key_sz_t)strlen(key1));
                FTU_ASSERT(iter.value_sz == (value_sz_t)strlen(value1));
                FTU_ASSERT(0 == strcmp(key1, iter.key));

                int ret = fhash_rehash(phash, 20);
                FTU_ASSERT(ret == 1);
            } else if (strcmp(data, value2) == 0) {
                value2_exist = 1;

                FTU_ASSERT(iter.key_sz == (key_sz_t)strlen(key2));
                FTU_ASSERT(iter.value_sz == (value_sz_t)strlen(value2));
                FTU_ASSERT(0 == strcmp(key2, iter.key));

                int ret = fhash_rehash(phash, 20);
                FTU_ASSERT(ret == 1);
            } else if (strcmp(data, value3) == 0) {
                value3_exist = 1;

                FTU_ASSERT(iter.key_sz == (key_sz_t)strlen(key3));
                FTU_ASSERT(iter.value_sz == (value_sz_t)strlen(value3));
                FTU_ASSERT(0 == strcmp(key3, iter.key));

                int ret = fhash_rehash(phash, 20);
                FTU_ASSERT(ret == 1);
            } else if (strcmp(data, value4) == 0) {
                value4_exist = 1;

                FTU_ASSERT(iter.key_sz == (key_sz_t)strlen(key4));
                FTU_ASSERT(iter.value_sz == (value_sz_t)strlen(value4));
                FTU_ASSERT(0 == strcmp(key4, iter.key));

                int ret = fhash_rehash(phash, 20);
                FTU_ASSERT(ret == 1);
            }
        }

        FTU_ASSERT(value1_exist == 1);
        FTU_ASSERT(value2_exist == 1);
        FTU_ASSERT(value3_exist == 1);
        FTU_ASSERT(value4_exist == 1);
        FTU_ASSERT(loop_cnt == 4);

        fhash_iter_release(&iter);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 4);
        FTU_ASSERT(phash->current->slots_used == 4);

        fhash_delete(phash);
    }

    // test reset the value of a key
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, strlen(key), value, strlen(value));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, strlen(key),
                                           &ret_value_sz);
        FTU_ASSERT(0 == strcmp(ret_value, value));
        FTU_ASSERT((size_t)ret_value_sz == strlen(value));

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        char value2[] = "test_value2";
        fhash_set(phash, key, strlen(key), value2, strlen(value2));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz2 = 0;
        char* ret_value2 = (char*)fhash_get(phash, key, strlen(key),
                                           &ret_value_sz2);
        FTU_ASSERT(0 == strcmp(ret_value2, value2));
        FTU_ASSERT((size_t)ret_value_sz2 == strlen(value2));

        fhash_delete(phash);
    }

    // test set a empty key or empty value
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key[] = "";
        char value[] = "test_value";
        fhash_set(phash, key, strlen(key), value, strlen(value));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 0);
        FTU_ASSERT(phash->current->slots_used == 0);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, strlen(key),
                                           &ret_value_sz);
        FTU_ASSERT(ret_value == NULL);
        FTU_ASSERT(ret_value_sz == 0);

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 0);
        FTU_ASSERT(phash->current->slots_used == 0);

        char key2[] = "test_key2";
        char value2[] = "";
        fhash_set(phash, key2, strlen(key2), value2, strlen(value2));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 0);
        FTU_ASSERT(phash->current->slots_used == 0);

        value_sz_t ret_value_sz2 = 0;
        char* ret_value2 = (char*)fhash_get(phash, key, strlen(key),
                                           &ret_value_sz2);
        FTU_ASSERT(ret_value2 == NULL);
        FTU_ASSERT(ret_value_sz2 == 0);

        fhash_delete(phash);
    }

    // test delete a empty key
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, strlen(key), value, strlen(value));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, strlen(key),
                                           &ret_value_sz);
        FTU_ASSERT(strcmp(ret_value, value) == 0);
        FTU_ASSERT((size_t)ret_value_sz == strlen(value));

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        char key2[] = "";
        fhash_del(phash, key2, strlen(key2));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        fhash_delete(phash);
    }

    // test delete a non-exist key
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, strlen(key), value, strlen(value));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, strlen(key),
                                           &ret_value_sz);
        FTU_ASSERT(strcmp(ret_value, value) == 0);
        FTU_ASSERT((size_t)ret_value_sz == strlen(value));

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        char key2[] = "test_key2";
        fhash_del(phash, key2, strlen(key2));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        fhash_delete(phash);
    }

    // test get a non-exist key
    {
        fhash_opt opt;
        opt.hash_alg = NULL;
        opt.compare = hash_core_compare;
        fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

        char key[] = "test_key";
        char value[] = "test_value";
        fhash_set(phash, key, strlen(key), value, strlen(value));
        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        value_sz_t ret_value_sz = 0;
        char* ret_value = (char*)fhash_get(phash, key, strlen(key),
                                           &ret_value_sz);
        FTU_ASSERT(strcmp(ret_value, value) == 0);
        FTU_ASSERT((size_t)ret_value_sz == strlen(value));

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        char key2[] = "test_key2";
        value_sz_t new_valuesz = 0;
        char* data = (char*)fhash_get(phash, key2, strlen(key2), &new_valuesz);
        FTU_ASSERT(new_valuesz == 0);
        FTU_ASSERT(data == NULL);

        FTU_ASSERT(phash->temporary == NULL);
        FTU_ASSERT(phash->iter_refs == 0);
        FTU_ASSERT(phash->current->index_size == 10);
        FTU_ASSERT(phash->current->index_used == 1);
        FTU_ASSERT(phash->current->slots_used == 1);

        fhash_delete(phash);
    }
}
