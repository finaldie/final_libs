#include <assert.h>
#include <stdlib.h>

#include "fhash_int.h"

// fhash_int is the same as fhash
struct fhash_int {
    fhash* phash;
    void*  ud;
};

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
fhash_int* fhash_int_create(uint32_t init_size, void* ud, uint32_t flags)
{
    fhash_opt opt;
    opt.hash_alg = NULL;
    opt.compare = _hash_int_compare;

    fhash* phash = fhash_create(init_size, opt, ud, flags);
    fhash_int* hash_int = malloc(sizeof(*hash_int));
    hash_int->phash = phash;
    hash_int->ud = ud;
    return hash_int;
}

void fhash_int_delete(fhash_int* hash_int)
{
    fhash_delete(hash_int->phash);
    free(hash_int);
}

void fhash_int_set(fhash_int* hash_int, int key, void* value)
{
    fhash_set(hash_int->phash, (void*)&key, sizeof(key), &value, sizeof(value));
}

void* fhash_int_get(fhash_int* hash_int, int key)
{
    void** value = (void**)fhash_get(hash_int->phash, (void*)&key, sizeof(key),
                                     NULL);
    if (value) {
        return *value;
    } else {
        return NULL;
    }
}

void* fhash_int_del(fhash_int* hash_int, int key)
{
    void* value = NULL;
    fhash_fetch_and_del(hash_int->phash, (void*)&key, sizeof(key),
                        &value, sizeof(value));
    return value;
}

fhash_int_iter fhash_int_iter_new(fhash_int* hash_int)
{
    fhash_int_iter iter;
    iter.iter = fhash_iter_new(hash_int->phash);
    iter.key = 0;
    iter.value = NULL;

    return iter;
}

void fhash_int_iter_release(fhash_int_iter* iter)
{
    fhash_iter_release(&iter->iter);
}

void* fhash_int_next(fhash_int_iter* iter)
{
    void* data = fhash_next(&iter->iter);
    if (!data) {
        return NULL;
    }

    iter->key = *(int*)iter->iter.key;
    iter->value = iter->iter.value;
    return iter->value;
}

void fhash_int_foreach(fhash_int* hash_int, fhash_int_each_cb cb)
{
    fhash_int_iter iter = fhash_int_iter_new(hash_int);

    void* data = NULL;
    while ((data = fhash_int_next(&iter))) {
        if (cb(hash_int->ud, iter.key, iter.value)) {
            break;
        }
    }

    fhash_int_iter_release(&iter);
}
