//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tu_inc.h"
#include "lhash.h"
#include "inc.h"

//TODO...


#define LOOP 10000
void test_hash(){
	f_hash* phash = hash_create(0);
	/*
	int n = 1;

	printf("before set, hash count=%d\n", hash_get_count(ph));
	hash_set_int(ph, 1, &n);
	hash_set_int(ph, 1, &n);
	printf("hash value = %d\n", *(int*)hash_get_int(ph, 1));
	printf("after set, hash count=%d\n", hash_get_count(ph));
	hash_del_int(ph, 1);
	printf("after del, hash count=%d\n", hash_get_count(ph));

	//printf("hash value = %d\n", *(int*)hash_get_int(ph, 1));
	*/
	
	int i;
	char test[20];
	//char va[20];
	for( i=0; i<LOOP; ++i )
	{
		//memset(test, 0, 20);
		//memset(va, 0, 20);
		char* va = (char*)malloc(20);

		sprintf(test, "test%d", i);
		sprintf(va, "a%d", i);
		hash_set_str(phash, test, va);
		char* res = (char*)hash_get_str(phash, test);
        FTU_ASSERT_EQUAL_CHAR(res, va);
	}

	for( i=0; i<LOOP; ++i )
	{
		//memset(test, 0, 20);
		//memset(va, 0, 20);
		char* va = (char*)malloc(20);
		sprintf(test, "www%d", i);
		sprintf(va, "ba%d", i);
		hash_set_str(phash, test, va);
		char* res = (char*)hash_get_str(phash, test);
        FTU_ASSERT_EQUAL_CHAR(res, va);
	}

	for( i=0; i<LOOP; ++i )
	{
		//memset(test, 0, 20);
		//memset(va, 0, 20);
		char* value = (char*)malloc(10);
		sprintf(value, "ca%d", i);
		hash_set_int(phash, i, value);
		char* res = (char*)hash_get_int(phash, i);
		assert(res);
        FTU_ASSERT_EQUAL_CHAR(res, value);
	}

	char* res = (char*)hash_get_int(phash, 0);
	assert(res);
    FTU_ASSERT_EQUAL_CHAR(res, "ca0");

	hash_statistics(phash);
	
	hiter iter = hash_iter(phash);
	void* data = NULL;
	int iter_count = 0;
	while( (data = hash_next(&iter)) ){
		//printf("iter data = %s\n", (char*)data);
		iter_count++;
	}
	printf("hash iter totoal=%d\n", iter_count);
    FTU_ASSERT_EQUAL_INT((3*LOOP), iter_count);

	int total_count = 0;
	int	test_print(void* data){
		//printf("key=%s(%d), value=%s\n", key, hash_atoi(key), (char*)data);
		total_count++;
		return 0;
	}

	hash_foreach(phash, test_print);
	printf("hashforeach totoal=%d\n", total_count);
    FTU_ASSERT_EQUAL_INT((3*LOOP), total_count);

	hash_delete(phash);
}

void	test_hash_del()
{
	f_hash* phash = hash_create(0);
	/*
	hash_set_int(phash, 1, "1");
	hash_set_int(phash, 2, "2");
	hash_set_int(phash, 3, "3");
	char* str = hash_del_int(phash, 1);
	assert(strcmp(str, "1") == 0);

	str = hash_del_int(phash, 2);
	assert(strcmp(str, "2") == 0);

	str = hash_del_int(phash, 3);
	assert(strcmp(str, "3") == 0);

	int i;
	for( i=0; i<10000; ++i )
	{
		int n = i - 5000;
		char* key = (char*)malloc(30);
		char* pkey = hash_itoa(n, key);
		hash_set_int(phash, n, pkey);
		char* res = (char*)hash_get_int(phash, n);
		assert(!strcmp(res, pkey));
	}

	int del_count = 0;
	int test_del(void* data)
	{
		char* s = (char*)hash_del_int(phash, k);
		if( hash_get_int(phash, k) ){
			printf("error! find key=%d\n", k);
		}
		char buf[30];
		char* _key = hash_itoa(k, buf);
		char tmp[20];
		sprintf("s=%s key=%s", s, _key);
		FASSERT( strcmp(s, _key) == 0, tmp);
		free(s);
		del_count++;
		return 0;
	}

	hash_foreach(phash, test_del);

	printf("hash del count = %d\n", del_count);
	*/

	int i;
	for( i=0; i<10000; ++i )
	{
		int n = i;
		char* key = (char*)malloc(30);
		char* pkey = hash_itoa(n, key);
		hash_set_int(phash, n, pkey);
		char* res = (char*)hash_get_int(phash, n);
		assert(!strcmp(res, pkey));
        FTU_ASSERT_EQUAL_CHAR(pkey, res);
	}

	for(i=0; i<10000; ++i){
		char* s = hash_del_int(phash, i);

		char buf[30];
		char* _key = hash_itoa(i, buf);
        FTU_ASSERT_EQUAL_CHAR(_key, s);
		free(s);

		assert( !hash_del_int(phash, i));
	}

	printf("hash del complete\n");
}

/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/
