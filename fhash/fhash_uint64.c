#include <assert.h>

#include "fhash_uint64.h"

// INTERNAL
static
int _hash_uint64_compare(const void* key1, key_sz_t key_sz1,
                      const void* key2, key_sz_t key_sz2)
{
    assert(key_sz1 == key_sz2 && key_sz1 == sizeof(uint64_t));
    uint64_t ikey1 = *(uint64_t*)key1;
    uint64_t ikey2 = *(uint64_t*)key2;

    return !(ikey1 == ikey2);
}

fhash* fhash_u64_create(uint32_t init_size, uint32_t flags)
{
    fhash_opt opt;
    opt.hash_alg = NULL;
    opt.compare = _hash_uint64_compare;

    return fhash_create(init_size, opt, NULL, flags);
}

void   fhash_u64_delete(fhash* phash)
{
    fhash_delete(phash);
}

void   fhash_u64_set(fhash* phash, uint64_t key, void* value)
{
    fhash_set(phash, (void*)&key, sizeof(key), &value, sizeof(value));
}

void*  fhash_u64_get(fhash* phash, uint64_t key)
{
    void** value = fhash_get(phash, (void*)&key, sizeof(key), NULL);
    if (value) {
        return *value;
    } else {
        return NULL;
    }
}

void*  fhash_u64_del(fhash* phash, uint64_t key)
{
    void* value = NULL;
    fhash_fetch_and_del(phash, (void*)&key, sizeof(key),
                        &value, sizeof(value));
    return value;
}

fhash_u64_iter fhash_u64_iter_new(fhash* phash)
{
    fhash_u64_iter iter;
    iter.iter = fhash_iter_new(phash);
    iter.key = 0;
    iter.value = NULL;

    return iter;
}

void fhash_u64_iter_release(fhash_u64_iter* iter)
{
    fhash_iter_release(&iter->iter);
}

void* fhash_u64_next(fhash_u64_iter* iter)
{
    void* data = fhash_next(&iter->iter);
    if (!data) {
        return NULL;
    }

    iter->key = *(uint64_t*)iter->iter.key;
    iter->value = *(void**)iter->iter.value;
    return iter->value;
}

void fhash_u64_foreach(fhash* phash, fhash_u64_each_cb cb, void* ud)
{
    fhash_u64_iter iter = fhash_u64_iter_new(phash);

    void* data = NULL;
    while ((data = fhash_u64_next(&iter))) {
        if (cb(ud, iter.key, iter.value)) {
            break;
        }
    }

    fhash_u64_iter_release(&iter);
}
