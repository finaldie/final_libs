//create by final
//desc: f_hash (unthread safe)

#ifndef _HASH_H_FINAL_
#define _HASH_H_FINAL_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _f_hash f_hash;

typedef struct {
    f_hash* phash;
    int     offset;
    int     idx;
}hiter;

// call back return 0 to continue iter else break out
typedef int (*pfunc_iter)(void*);

f_hash* hash_create(int size);
void    hash_delete(f_hash*);

void    hash_set_int(f_hash*, int key, void* value);
void    hash_set_str(f_hash*, const char* key, void* value);

void*   hash_get_int(f_hash*, int key);
void*   hash_get_str(f_hash*, const char* key);

void*   hash_del_int(f_hash*, int key);
void*   hash_del_str(f_hash*, const char* key);

hiter   hash_iter(f_hash*);
void*   hash_next(hiter*);

void    hash_foreach(f_hash*, pfunc_iter);
int     hash_atoi(char* key);
char*   hash_itoa(int v, char* retbuff);

// return total value count
int     hash_get_count(f_hash*);
void    hash_statistics(f_hash* phash);

#ifdef __cplusplus
}
#endif

#endif
