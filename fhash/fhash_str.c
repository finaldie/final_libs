#include <assert.h>
#include <string.h>

#include "fhash.h"

// INTERNAL
static
int _hash_str_compare(const void* key1, key_sz_t key_sz1,
                      const void* key2, key_sz_t key_sz2)
{
    const char* skey1 = (const char*)key1;
    const char* skey2 = (const char*)key2;

    return strncmp(skey1, skey2, key_sz1);
}

// API
fhash* fhash_str_create(uint32_t init_size, void* ud, uint32_t flags)
{
    fhash_opt opt;
    opt.hash_alg = NULL;
    opt.compare = _hash_str_compare;

    return fhash_create(init_size, opt, ud, flags);
}

void   fhash_str_delete(fhash* phash)
{
    fhash_delete(phash);
}

void   fhash_str_set(fhash* phash, const void* key, void* value)
{
    fhash_set(phash, key, strlen(key), &value, sizeof(value));
}

void*  fhash_str_get(fhash* phash, const void* key)
{
    void** value = (void**)fhash_get(phash, key, strlen(key), NULL);
    if (value) {
        return *value;
    } else {
        return NULL;
    }
}

void*  fhash_str_del(fhash* phash, const void* key)
{
    void* value = NULL;
    fhash_fetch_and_del(phash, key, strlen(key),
                                        &value, sizeof(value));
    return value;
}
