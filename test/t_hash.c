//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "tu_inc.h"
#include "lhash.h"
#include "inc.h"

//TODO...


#define LOOP 10000
void test_hash(){
    f_hash* phash = hash_create(0);

    int i;
    char test[20];
    for ( i=0; i<LOOP; ++i ) {
        char* va = (char*)malloc(20);

        sprintf(test, "test%d", i);
        sprintf(va, "a%d", i);
        hash_set_str(phash, test, va);
        char* res = (char*)hash_get_str(phash, test);
        FTU_ASSERT_EQUAL_CHAR(res, va);
        free(va);
    }

    for ( i=0; i<LOOP; ++i ) {
        char* va = (char*)malloc(20);
        sprintf(test, "www%d", i);
        sprintf(va, "ba%d", i);
        hash_set_str(phash, test, va);
        char* res = (char*)hash_get_str(phash, test);
        FTU_ASSERT_EQUAL_CHAR(res, va);
        free(va);
    }

    for ( i=0; i<LOOP; ++i ) {
        char* value = (char*)malloc(10);
        sprintf(value, "ca%d", i);
        hash_set_int(phash, i, value);
        char* res = (char*)hash_get_int(phash, i);
        assert(res);
        FTU_ASSERT_EQUAL_CHAR(res, value);
        free(value);
    }

    printf("uint64 testing\n");
    uint64_t j = 0;
    for ( j=LOOP*10; j<LOOP*11; ++j ) {
        char* value = (char*)malloc(10);
        sprintf(value, "uint64_%" PRIu64 , j);
        hash_set_uint64(phash, j, value);
        char* res = (char*)hash_get_uint64(phash, j);
        assert(res);
        FTU_ASSERT_EQUAL_CHAR(res, value);
        free(value);
    }
    printf("uint64 testing end\n");

    hash_statistics(phash);

    hiter iter = hash_iter(phash);
    void* data = NULL;
    int iter_count = 0;
    while ( (data = hash_next(&iter)) ) {
        //printf("iter data = %s\n", (char*)data);
        iter_count++;
    }
    printf("hash iter totoal=%d\n", iter_count);
    FTU_ASSERT_EQUAL_INT((4*LOOP), iter_count);

    int total_count = 0;
    int test_print(void* data) {
        //printf("key=%s(%d), value=%s\n", key, hash_atoi(key), (char*)data);
        total_count++;
        return 0;
    }

    hash_foreach(phash, test_print);
    printf("hashforeach totoal=%d\n", total_count);
    FTU_ASSERT_EQUAL_INT((4*LOOP), total_count);

    hash_delete(phash);
}

void    test_hash_del()
{
    f_hash* phash = hash_create(0);

    int i;
    for ( i=0; i<10000; ++i ) {
        int n = i;
        char* key = (char*)malloc(30);
        char* pkey = hash_itoa(n, key);
        hash_set_int(phash, n, pkey);
        char* res = (char*)hash_get_int(phash, n);
        assert(!strcmp(res, pkey));
        FTU_ASSERT_EQUAL_CHAR(pkey, res);
    }

    for (i=0; i<10000; ++i) {
        char* s = hash_del_int(phash, i);

        char buf[30];
        char* _key = hash_itoa(i, buf);
        FTU_ASSERT_EQUAL_CHAR(_key, s);
        free(s);

        assert( !hash_del_int(phash, i));
    }

    hash_delete(phash);

    printf("hash del complete\n");
}
