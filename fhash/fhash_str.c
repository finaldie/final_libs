#include <assert.h>
#include <string.h>

#include "fhash_str.h"

// INTERNAL
static
int _hash_str_compare(const void* key1, key_sz_t key_sz1,
                      const void* key2, key_sz_t key_sz2)
{
    if (key_sz1 != key_sz2) {
        return 1;
    }

    const char* skey1 = (const char*)key1;
    const char* skey2 = (const char*)key2;
    return strncmp(skey1, skey2, key_sz1);
}

// API
fhash* fhash_str_create(uint32_t init_size, uint32_t flags)
{
    fhash_opt opt;
    opt.hash_alg = NULL;
    opt.compare = _hash_str_compare;

    return fhash_create(init_size, opt, flags);
}

void   fhash_str_delete(fhash* phash)
{
    fhash_delete(phash);
}

void   fhash_str_set(fhash* phash, const char* key, void* value)
{
    if (!key || !value) {
        return;
    }

    fhash_set(phash, key, strlen(key), &value, sizeof(value));
}

void*  fhash_str_get(fhash* phash, const char* key)
{
    void** value = fhash_get(phash, key, strlen(key), NULL);
    if (value) {
        return *value;
    } else {
        return NULL;
    }
}

void*  fhash_str_del(fhash* phash, const char* key)
{
    void* value = NULL;
    fhash_fetch_and_del(phash, key, strlen(key),
                        &value, sizeof(value));
    return value;
}

fhash_str_iter fhash_str_iter_new(fhash* phash)
{
    fhash_str_iter iter;
    iter.iter = fhash_iter_new(phash);
    iter.key = NULL;
    iter.value = NULL;

    return iter;
}

void fhash_str_iter_release(fhash_str_iter* iter)
{
    fhash_iter_release(&iter->iter);
}

void* fhash_str_next(fhash_str_iter* iter)
{
    void* data = fhash_next(&iter->iter);
    if (!data) {
        return NULL;
    }

    iter->key = iter->iter.key;
    iter->value = *(void**)iter->iter.value;
    return iter->value;
}

void fhash_str_foreach(fhash* phash, fhash_str_each_cb cb, void* ud)
{
    fhash_str_iter iter = fhash_str_iter_new(phash);

    void* data = NULL;
    while ((data = fhash_str_next(&iter))) {
        if (cb(ud, iter.key, iter.value)) {
            break;
        }
    }

    fhash_str_iter_release(&iter);
}
