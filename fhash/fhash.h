//create by final
//desc: fhash (unthread safe)

#ifndef _HASH_H_FINAL_
#define _HASH_H_FINAL_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct _f_hash fhash;

typedef struct {
    fhash*  phash;
    int     offset;
    int     idx;
} fhash_iter;

// call back return 0 to continue iter else break out
typedef int (*fhash_each_cb)(void*);

fhash*     fhash_create(int size);
void       fhash_delete(fhash*);

void       fhash_set_int(fhash*, int key, void* value);
void*      fhash_get_int(fhash*, int key);
void*      fhash_del_int(fhash*, int key);

void       fhash_set_uint64(fhash*, uint64_t key, void* value);
void*      fhash_get_uint64(fhash*, uint64_t key);
void*      fhash_del_uint64(fhash*, uint64_t key);

void       fhash_set_str(fhash*, const char* key, void* value);
void*      fhash_get_str(fhash*, const char* key);
void*      fhash_del_str(fhash*, const char* key);

fhash_iter fhash_new_iter(fhash*);
void*      fhash_next(fhash_iter*);

void       fhash_foreach(fhash*, fhash_each_cb);
int        fhash_atoi(const char* key);
char*      fhash_itoa(int v, char* retbuff);

// return total value count
int        fhash_get_count(fhash*);
void       fhash_statistics(fhash* phash);

#ifdef __cplusplus
}
#endif

#endif
