//create by final
//desc : f_hash

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "lhash.h"

#define DEFAULT_SIZE        10
#define DEFAULT_LIST_SIZE   4
#define MAX_INT_KEY_SIZE    22
#define MAGIC_PRIME         16777619

#define HASH_TYPE_INT       0
#define HASH_TYPE_STR       1

typedef unsigned long       _ulong;

#pragma pack(1)

typedef char htype;
typedef union{
    char* str_key;
    int   int_key;
}hkey;

typedef struct
{
    htype type;
    hkey  key;
    void* value;
}f_hash_node;

typedef struct
{
    int   size;
    int   max_size;
    f_hash_node* hash_list;
}hash_mgr;

struct _f_hash
{
    int size;
    int value_count;

    hash_mgr ph_mgr[0];
};
#pragma pack()

static
_ulong    base_str_hash(const char* str)
{
    register int i,l;
    register unsigned long ret=0;
    register unsigned short *s; 

    if (str == NULL) return(0);

    l = (strlen(str)+1) / 2;
    s = (unsigned short *)str; 

    for (i=0; i<l; ++i)
        ret ^= (s[i]<<(i & 0x0f));

    return ret;
}

//static
_ulong    base_str_hash1(char* c)
{
    _ulong ret = 0, v;
    long n;
    int r;

    if ((c == NULL) || (*c == '\0'))
        return ret;
    /*
     * unsigned char b[16];
     * MD5(c,strlen(c),b);
     * return(b[0]|(b[1]<<8)|(b[2]<<16)|(b[3]<<24));
     * */

    n=0x100;
    while(*c)
    {
        v = n | (*c);
        n += 0x100;
        r = (int)((v>>2) ^ v) & 0x0f;
        ret = ret & (32 - r);
        ret &= 0xFFFFFFFFL;
        ret ^= v * v;
        c++;
    }

    return ((ret >> 16) ^ ret);
}

// Don't recommend to use it
static inline
int     base_int_hash(int key)
{
    return (key + MAGIC_PRIME);
}

static inline
int        hash_str(f_hash* phash, const char* key)
{
    return base_str_hash(key) % phash->size;
}

static inline
int        hash_int(f_hash* phash, int key)
{
    return base_int_hash(key) % phash->size;
}

static inline
char*    make_key(const char* key)
{
    int len = strlen(key) + 1;
    char* pkey = malloc(len);
    return strncpy(pkey, key, len);
}

static inline
f_hash_node*    make_node(f_hash_node* pnode, int type, const void* key, void* value)
{
    pnode->type = (htype)type;

    if( type == HASH_TYPE_INT )
        pnode->key.int_key = *(int*)key;
    else
        pnode->key.str_key = make_key(key);

    pnode->value = value;

    return pnode;
}

// hex str
static inline
char*    f_itoa(int v, char* retbuff)
{
    sprintf(retbuff, "%x", v);
    return retbuff;
}

char*     hash_itoa(int v, char* retbuff)
{
    sprintf(retbuff, "%d", v);
    return retbuff;
}

static inline
int        f_atoi(char* v){
    return (int)strtol(v, (char**)NULL, 16);
}

inline
int        hash_atoi(char* v){
    return f_atoi(v);
}

static inline
hash_mgr* hash_get_head(f_hash* phash, int h)
{
    return &phash->ph_mgr[h];
}

static inline
f_hash_node* hash_get_list(f_hash* phash, int h)
{
    return hash_get_head(phash, h)->hash_list;
}

static inline
void    hash_set_list(f_hash* phash, int h, f_hash_node* plist)
{
    hash_get_head(phash, h)->hash_list = plist;
}

static inline
int        hash_get_size(f_hash* phash, int h)
{
    return hash_get_head(phash, h)->size;
}

static inline
int        hash_get_maxsize(f_hash* phash, int h)
{
    return hash_get_head(phash, h)->max_size;
}

static inline
f_hash_node* hash_create_list(size_t max_size)
{
    return (f_hash_node*)malloc( sizeof(f_hash_node) * max_size );
}

static
void    hash_inc_list(hash_mgr* pmgr)
{
    int size = pmgr->size;

    int new_size = (size << 1) - (size >> 1);
    pmgr->max_size = new_size;
    pmgr->hash_list = (f_hash_node*)realloc(pmgr->hash_list, sizeof(f_hash_node) * new_size);
}

static
f_hash_node*    hash_find(f_hash* phash, int h, int type, const void* key)
{
    f_hash_node* node = hash_get_list(phash, h);
    int size = hash_get_size(phash, h);
    int i;

    for( i = 0; i < size && node != NULL; i++, node += 1 )
    {
        if( type == HASH_TYPE_INT ){
            if( node->key.int_key != *(int*)key )
                continue;
        } else{
            if( strcmp(key, node->key.str_key) != 0 )
                continue;
        }

        return node;
    }

    return NULL;
}

static
void    hash_add(f_hash* phash, int h, int type, const void* key, void* value)
{
    hash_mgr* pmgr = hash_get_head(phash, h);
    int size = pmgr->size;
    int max_size = pmgr->max_size;
    if( size == max_size )
        hash_inc_list(pmgr);

    f_hash_node* plist = pmgr->hash_list;
    f_hash_node* pnew = plist + size;
    make_node(pnew, type, key, value);
    pmgr->size += 1;
    phash->value_count += 1;
}

static
void     hash_set(f_hash* phash, int h, int type, const void* key, void* value)
{
    hash_mgr* mgr = hash_get_head(phash, h);
    f_hash_node* plist = hash_get_list(phash, h);
    if( !plist ){
        plist = hash_create_list(DEFAULT_LIST_SIZE);
        hash_set_list(phash, h, plist);
        mgr->max_size = DEFAULT_LIST_SIZE;

        hash_add(phash, h, type, key, value);
        return;
    }

    f_hash_node* pnode = hash_find(phash, h, type, key);
    if( pnode ){
        pnode->value = value;
    } else
        hash_add(phash, h, type, key, value);
}

static inline
void*    hash_get(f_hash* phash, int h, int type, const void* key)
{
    f_hash_node* pnode = hash_find(phash, h, type, key);
    if( pnode ){
        return pnode->value;
    }

    return NULL;
}

static inline
void*    hash_del(f_hash* phash, int h, int type, const void* key)
{
    f_hash_node* node = hash_get_list(phash, h);
    if( !node ) return NULL;
    int size = hash_get_size(phash, h);
    int i;

    int find = 0;
    for( i = 0; i < size; i++, node += 1 )
    {
        if( type == HASH_TYPE_INT ){
            if( node->key.int_key == *(int*)key ){
                find = 1;
            }
        } else{
            if( strcmp((const char*)key, node->key.str_key) == 0 ){
                free(node->key.str_key);
                find = 1;
            }
        }

        if( find ){
            void* ret_value = node->value;
            if( i < size - 1 ){
                int left_cnt = size -1 - i;
                memmove(node, node + 1, sizeof(f_hash_node) * left_cnt );
            }

            hash_get_head(phash, h)->size--;
            phash->value_count--;
            return ret_value;
        }
    }

    return NULL;
}

//-------------------open api-----------------------------------
f_hash*    hash_create(int size)
{
    int f_size = 0;
    if( size <= 0 )
        f_size = DEFAULT_SIZE;
    else
        f_size = size;

    f_hash* phash = (f_hash*)malloc(sizeof(f_hash) + sizeof(hash_mgr) * f_size);

    phash->size = f_size;
    phash->value_count = 0;
    int i;
    for(i=0; i<f_size; ++i)
    {
        phash->ph_mgr[i].size = 0;
        phash->ph_mgr[i].max_size = 0;
        phash->ph_mgr[i].hash_list = NULL;
    }

    return phash;
}

void    hash_delete(f_hash* phash)
{
    if ( !phash ) return;
    int i, size = phash->size;

    for(i=0; i<size; ++i)
    {
        f_hash_node* plist = hash_get_list(phash, i);
        int list_size = hash_get_size(phash, i);
        f_hash_node* node = plist;
        int i;

        for( i = 0; i < list_size && node != NULL; i++, node += 1 ){
            if( node->type == HASH_TYPE_STR )
                free(node->key.str_key);
        }

        free(plist);
    }

    free( phash );
}

inline
void    hash_set_int(f_hash* phash, int key, void* value)
{
    char new_key[MAX_INT_KEY_SIZE];
    f_itoa(key, new_key);
    int h = hash_str(phash, new_key);

    hash_set(phash, h, HASH_TYPE_INT, &key, value);
}

inline
void    hash_set_str(f_hash* phash, const char* key, void* value)
{
    int h = hash_str(phash, key);

    hash_set(phash, h, HASH_TYPE_STR, key, value);
}

inline
void*    hash_get_int(f_hash* phash, int key)
{
    char new_key[MAX_INT_KEY_SIZE];
    f_itoa(key, new_key);
    int h = hash_str(phash, new_key);

    return hash_get(phash, h, HASH_TYPE_INT, &key);
}

inline
void*    hash_get_str(f_hash* phash, const char* key)
{
    int h = hash_str(phash, key);

    return hash_get(phash, h, HASH_TYPE_STR, key);
}

void*    hash_del_int(f_hash* phash, int key)
{
    char new_key[MAX_INT_KEY_SIZE];
    f_itoa(key, new_key);
    int h = hash_str(phash, new_key);

    return hash_del(phash, h, HASH_TYPE_INT, &key);
}

void*    hash_del_str(f_hash* phash, const char* key)
{
    int h = hash_str(phash, key);
    return hash_del(phash, h, HASH_TYPE_STR, key);
}

hiter hash_iter(f_hash* phash)
{
    hiter iter;

    iter.phash = phash;
    iter.offset = 0;
    iter.idx = 0;

    return iter;
}

void*     hash_next(hiter* iter)
{
    f_hash* phash = iter->phash;
    int size = phash->size;
    if( iter->offset >= size ) return NULL;

    hash_mgr* pmgr = hash_get_head(phash, iter->offset);
    int list_size = pmgr->size;
    if( iter->idx < list_size ){
        f_hash_node* plist = hash_get_list(phash, iter->offset);
        f_hash_node* pnode = plist + iter->idx;
        iter->idx++;

        return pnode->value;
    } else{
        iter->offset += 1;
        iter->idx = 0;

        if( iter->offset >= size )
            return NULL;
        else
            return hash_next(iter);
    }
}

void    hash_foreach(f_hash* phash, pfunc_iter pfunc){
    int i, j, size = phash->size;

    for(i=0; i<size; ++i){
        f_hash_node* plist = hash_get_list(phash, i);
        int list_size = hash_get_size(phash, i);

        if( list_size == 0 )
            continue;

        for(j=list_size-1; j>=0; ){
            f_hash_node* node = plist + j;
            if( pfunc(node->value) )
                break;

            int new_size = hash_get_size(phash, i);
            if( new_size == 0 ) break;
            if( new_size == list_size )
                j--;
            else{
                j = new_size - 1;
                list_size = new_size;
            }
        }
    }
}

int        hash_get_count(f_hash* phash)
{
    return phash->value_count;
}

void    hash_statistics(f_hash* phash){
    int i = 0, count = 0, use = 0, max = 0;
    for( i=0; i<phash->size; ++i )
    {
        hash_mgr* pmgr = hash_get_head(phash, i);

        if( pmgr->size > 0 )
        {
            printf("%d--%d/%d\n", i, pmgr->size, pmgr->max_size);
            count += pmgr->size;
            use++;
        }

        max += pmgr->max_size;
    }

    printf("total count = %d max_size = %d(cost %d byte) use ratio=%.2f avg len=%d\n", 
        count, max, (int)(sizeof(f_hash_node) * max), ((float)use/(float)phash->size), use == 0?0:count/use);
}

/*
// gcc -Wall -g -o hash lhash.c list.c
int main(int argc, char** argv)
{
    f_hash* phash = hash_create(0);
    //hash_set_int(phash, 10, "abc");
    //hash_set_str(phash, "test", "efg");

    //void* data1 = hash_get_int(phash, 10);
    //void* data2 = hash_get_str(phash, "test");
    //printf("data1 = %s\n", (char*)data1);
    //printf("data2 = %s\n", (char*)data2);

    int i;
    char test[20];
    for( i=0; i<200; ++i )
    {
        sprintf(test, "test%d", i);
        hash_set_str(phash, test, "efg");
        char* res = (char*)hash_get_str(phash, test);
        assert(!strcmp(res, "efg"));
    }

    for( i=0; i<200; ++i )
    {
        sprintf(test, "de%d", i);
        hash_set_str(phash, test, "efg1");
        char* res = (char*)hash_get_str(phash, test);
        assert(!strcmp(res, "efg1"));
    }

    for( i=0; i<200; ++i )
    {
        hash_set_int(phash, i-100, "efg2");
        char* res = (char*)hash_get_int(phash, i-100);
        assert(!strcmp(res, "efg2"));
    }

    hash_statistics(phash);

    int    test_print(char* key, void* data){
        //printf("key=%s(%d), value=%s\n", key, hash_atoi(key), (char*)data);
        return 0;
    }

    hash_foreach(phash, test_print);

    hash_delete(phash);

    return 0;
}
*/
