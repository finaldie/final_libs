#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "flibs/fhash_int.h"

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
fhash* fhash_int_create(uint32_t init_size, uint32_t flags)
{
    fhash_opt opt;
    opt.hash_alg = NULL;
    opt.compare = _hash_int_compare;

    return fhash_create(init_size, opt, flags);
}

void fhash_int_delete(fhash* phash)
{
    fhash_delete(phash);
}

void fhash_int_set(fhash* phash, int key, const void* value)
{
    if (!value) {
        return;
    }

    fhash_set(phash, (void*)&key, sizeof(key), &value, sizeof(value));
}

void* fhash_int_get(fhash* phash, int key)
{
    void** value = fhash_get(phash, (void*)&key, sizeof(key), NULL);
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

fhash_int_iter fhash_int_iter_new(fhash* phash)
{
    fhash_int_iter iter;
    memset(&iter, 0, sizeof(iter));
    iter.iter = fhash_iter_new(phash);
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
    iter->value = *(void**)iter->iter.value;
    return iter->value;
}

void fhash_int_foreach(fhash* phash, fhash_int_each_cb cb, void* ud)
{
    fhash_int_iter iter = fhash_int_iter_new(phash);

    void* data = NULL;
    while ((data = fhash_int_next(&iter))) {
        if (cb(ud, iter.key, iter.value)) {
            break;
        }
    }

    fhash_int_iter_release(&iter);
}
