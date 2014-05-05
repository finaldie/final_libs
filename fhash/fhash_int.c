#include <assert.h>

#include "fhash.h"

// INTERNAL
static
int _hash_int_compare(const void* key1, key_sz_t key_sz1,
                      const void* key2, key_sz_t key_sz2)
{
    assert(key_sz1 == key_sz2 && key_sz1 == sizeof(int));
    int ikey1 = *(int*)key1;
    int ikey2 = *(int*)key2;

    return !(ikey1 == ikey2);
}

// API
fhash* fhash_int_create(uint32_t init_size, void* ud, uint32_t flags)
{
    fhash_opt opt;
    opt.hash_alg = NULL;
    opt.compare = _hash_int_compare;

    return fhash_create(init_size, opt, ud, flags);
}

void fhash_int_delete(fhash* phash)
{
    fhash_delete(phash);
}

void fhash_int_set(fhash* phash, int key, void* value)
{
    fhash_set(phash, (void*)&key, sizeof(key), &value, sizeof(value));
}

void* fhash_int_get(fhash* phash, int key)
{
    void** value = (void**)fhash_get(phash, (void*)&key, sizeof(key), NULL);
    if (value) {
        return *value;
    } else {
        return NULL;
    }
}

void* fhash_int_del(fhash* phash, int key)
{
    void* value = NULL;
    fhash_fetch_and_del(phash, (void*)&key, sizeof(key),
                                    &value, sizeof(value));
    return value;
}
