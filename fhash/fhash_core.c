#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include "fhash_core.h"

typedef uint64_t data_sz_t;

#define DEFAULT_TABLE_SIZE  10
#define DEFAULT_LIST_SIZE   4
#define NODELIST_ENLARGE_COEFFICIENT 2

#define NODE_INVALID 0
#define NODE_VALID   1

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define AUTO_REHASH_THRESHOLD 4

//==========================internal macro functions============================
#define FHASH_DATASZ(key_sz, value_sz) \
    ((data_sz_t)key_sz + 1 + (data_sz_t)value_sz + 1)

#define FHASH_REHASH_SIZE(table) \
    ((table->index_size) <= UINT32_MAX / 2 \
        ? table->index_size * 2 \
        : UINT32_MAX)

// there is a '\0' after key and value, that will be friendly if the key or data
// is string
typedef struct _fhash_node {
    data_sz_t  real_sz;   // size of real memory space

    uint32_t valid:1;
    uint32_t padding:31;  // reserved

    key_sz_t   key_sz;
    value_sz_t value_sz;
    void*      data;

} _fhash_node;

typedef struct {
    size_t used;
    size_t size;
    _fhash_node* node_list;
} _fhash_node_mgr;

typedef struct _fhash {
    uint32_t index_size;
    uint32_t index_used;

    size_t   slots_used;
    _fhash_node_mgr node_mgr[0];
} _fhash;

typedef union {
    struct {
        // user flags
        uint32_t auto_rehash:1;
        uint32_t padding:29;    // reserved

        // internal use
        uint32_t rehashing:1;   // doing rehash
        uint32_t performing:1;  // doing delayed actions
    };

    uint32_t value;
} fhash_mask;

struct fhash {
    void*      ud;          // User Data
    uint32_t   iter_refs;   // how many iterators have already refenerced it

    fhash_mask mask;

    fhash_opt  opt;
    _fhash*    current;
    _fhash*    temporary;   // we use it when trigger resizing

    // a delay action list, only *ADD* action can add into it
    _fhash_node_mgr delayed_actions;
};

//===================================INTERNAL===================================
//--------------------------------HASH ALGORITHM--------------------------------

// The FNV-1 Algorithm
static
uint32_t _hash_fnv_algorithm(const void* data, key_sz_t data_sz)
{
    unsigned char* bp = (unsigned char*)data;  // start of buffer
    unsigned char* be = bp + data_sz;          // beyond end of buffer
    uint32_t hval = 0;

    while (bp < be) {
        // multiply by the 32 bit FNV magic prime mod 2^32
        hval *= FNV_32_PRIME;
        // xor the bottom with the current octet
        hval ^= (uint32_t)*bp++;
    }

    return hval;
}

//-------------------------------------NODE-------------------------------------
static
void _hash_node_init(_fhash_node* node)
{
    memset(node, 0, sizeof(*node));
}

static
void _hash_node_try_enlarge(_fhash_node* node,
                            key_sz_t key_sz, value_sz_t value_sz)
{
    data_sz_t new_data_sz = FHASH_DATASZ(key_sz, value_sz);
    if (new_data_sz > node->real_sz) {
        node->data = realloc(node->data, new_data_sz);
        node->real_sz = new_data_sz;
    }
}

static inline
void* _hash_node_key(_fhash_node* node)
{
    return node->data;
}

static inline
key_sz_t _hash_node_keysz(_fhash_node* node)
{
    return node->key_sz;
}

static inline
void _hash_node_set_key(_fhash_node* node, const void* key, key_sz_t key_sz)
{
    _hash_node_try_enlarge(node, key_sz, node->value_sz);

    memcpy(node->data, key, key_sz);
    *(char*)(node->data + key_sz) = '\0';
    node->key_sz = key_sz;
}

static inline
void* _hash_node_value(_fhash_node* node)
{
    return node->data + node->key_sz + 1;
}

static inline
value_sz_t _hash_node_valuesz(_fhash_node* node)
{
    return node->value_sz;
}

static inline
void _hash_node_set_value(_fhash_node* node,
                          const void* value, value_sz_t value_sz)
{
    _hash_node_try_enlarge(node, node->key_sz, value_sz);

    memcpy(_hash_node_value(node), value, value_sz);
    *(char*)(node->data + node->key_sz + 1 + value_sz) = '\0';
    node->value_sz = value_sz;
}

static
void _hash_node_delete(_fhash_node* node)
{
    node->valid = NODE_INVALID;
    free(node->data);
}

static
void _hash_node_set(_fhash_node* node,
                    const void* key, key_sz_t key_sz,
                    const void* value, value_sz_t value_sz)
{
    _hash_node_try_enlarge(node, key_sz, value_sz);
    node->valid = NODE_VALID;
    node->key_sz = key_sz;
    node->value_sz = value_sz;

    _hash_node_set_key(node, key, key_sz);
    _hash_node_set_value(node, value, value_sz);
}

//-----------------------------------NODE_MGR-----------------------------------
static inline
void _hash_nodemgr_init(_fhash_node_mgr* mgr)
{
    mgr->used = 0;
    mgr->size = 0;
    mgr->node_list = NULL;
}

static inline
void _hash_nodemgr_destroy(_fhash_node_mgr* mgr)
{
    _fhash_node* node = mgr->node_list;
    size_t list_size = mgr->size;

    for (size_t i = 0; i < list_size && node != NULL; i++, node += 1) {
        _hash_node_delete(node);
    }

    free(mgr->node_list);
}

static inline
size_t _hash_nodemgr_used(_fhash_node_mgr* mgr)
{
    return mgr->used;
}

static inline
size_t _hash_nodemgr_size(_fhash_node_mgr* mgr)
{
    return mgr->size;
}

static
_fhash_node* _hash_nodemgr_find(_fhash_node_mgr* mgr, fhash_opt* opt,
                                const void* key, key_sz_t key_sz, int target)
{
    size_t size = mgr->size;
    _fhash_node* node = mgr->node_list;

    for (size_t i = 0; i < size; i++, node += 1) {
        if (node->valid != target) {
            continue;
        }

        if (!opt->compare(_hash_node_key(node), node->key_sz, key, key_sz)) {
            return node;
        }
    }

    return NULL;
}

static
void _hash_nodemgr_enlarge(_fhash_node_mgr* mgr)
{
    size_t old_slots_size = mgr->size;
    size_t new_slots_size = mgr->size;
    if (new_slots_size == 0) {
        new_slots_size = 1;
    } else {
        new_slots_size *= NODELIST_ENLARGE_COEFFICIENT;
    }

    assert(new_slots_size);
    mgr->node_list = realloc(mgr->node_list,
                             sizeof(_fhash_node) * new_slots_size);
    // init the new slots
    for (size_t i = old_slots_size; i < new_slots_size; i++) {
        _hash_node_init(&mgr->node_list[i]);
    }

    mgr->size = new_slots_size;
}

static
_fhash_node* _hash_nodemgr_first(_fhash_node_mgr* mgr,
                                 int target, size_t* slot_idx)
{
    for (size_t i = 0; i < mgr->size; i++) {
        _fhash_node* node = &mgr->node_list[i];
        if (node->valid == target) {
            if (slot_idx) {
                *slot_idx = i;
            }
            return node;
        }
    }

    return NULL;
}

// try to find a node which the waste space is lowest
static
_fhash_node* _hash_nodemgr_applicable_node(_fhash_node_mgr* mgr, int target,
                                           key_sz_t key_sz, value_sz_t value_sz,
                                           size_t* slot_idx)
{
    // 1. try to find a node which the waste space is lowest
    _fhash_node* best_node = NULL;
    data_sz_t lowest_waste_sz = 0;
    size_t best_idx = 0;

    for (size_t i = 0; i < mgr->size; i++) {
        _fhash_node* node = &mgr->node_list[i];
        if (node->valid != target) {
            continue;
        }

        data_sz_t new_data_sz = FHASH_DATASZ(key_sz, value_sz);
        data_sz_t node_sz = node->real_sz;

        if (new_data_sz > node_sz) {
            continue;
        }

        data_sz_t waste_sz = node_sz - new_data_sz;
        // find one
        if (!best_node || waste_sz < lowest_waste_sz) {
            best_node = node;
            lowest_waste_sz = waste_sz;
            best_idx = i;
        }
    }

    if (best_node) {
        if (slot_idx) {
            *slot_idx = best_idx;
        }
        return best_node;
    }

    // 2. doesn't find one, return the first target node
    return _hash_nodemgr_first(mgr, target, slot_idx);
}

// enlarge the node list and add the new node into it
static
void _hash_nodemgr_add(_fhash_node_mgr* mgr,
                       const void* key, key_sz_t key_sz,
                       const void* value, value_sz_t value_sz)
{
    // 1. find the first available node(enlarge the node list if needed)
    // 2. fill the node data
    // 3. update used flag

    _fhash_node* node = NULL;

    // 1.
    if (mgr->used == mgr->size) {
        // no enough space, enlarge it
        size_t slot_idx = mgr->size;
        _hash_nodemgr_enlarge(mgr);
        node = &mgr->node_list[slot_idx];
    } else {
        node = _hash_nodemgr_applicable_node(mgr, NODE_INVALID,
                                             key_sz, value_sz, NULL);
        assert(node);
    }

    // 2.
    _hash_node_set(node, key, key_sz, value, value_sz);

    // 3.
    mgr->used++;
}

static
void  _hash_nodemgr_set(_fhash_node_mgr* mgr, _fhash_node* node,
                        const void* key, key_sz_t key_sz,
                        const void* value, value_sz_t value_sz)
{
    _hash_node_set(node, key, key_sz, value, value_sz);
}

static
void _hash_nodemgr_del(_fhash_node_mgr* mgr, _fhash_node* node)
{
    node->valid = NODE_INVALID;
    mgr->used--;
}

static
_fhash_node* _hash_nodemgr_next(_fhash_node_mgr* mgr, fhash_iter* iter,
                                int target)
{
    size_t slot_idx = iter->slot;

    _fhash_node* node = NULL;
    for (; slot_idx < mgr->size; slot_idx++) {
        node = &mgr->node_list[slot_idx];
        if (node->valid == target) {
            break;
        }
    }

    if (!node) {
        // reach the end, should reset the slot_idx
        iter->slot = 0;
        return NULL;
    }

    iter->slot = ++slot_idx;
    iter->key      = _hash_node_key(node);
    iter->key_sz   = _hash_node_keysz(node);
    iter->value    = _hash_node_value(node);
    iter->value_sz = _hash_node_valuesz(node);

    return node;
}

//------------------------------------TABLE-------------------------------------
static inline
uint32_t _hash_tbl_calculate_idx(_fhash* table, fhash_opt* opt,
                             const void* key, key_sz_t key_sz)
{
    uint32_t idx = opt->hash_alg(key, key_sz);
    return idx % table->index_size;
}

static
_fhash* _hash_tbl_create(uint32_t init_size)
{
    assert(init_size > 0);
    _fhash* table = calloc(1, sizeof(_fhash) +
                              sizeof(_fhash_node_mgr) * init_size);

    table->index_size = init_size;

    // init all node managers
    for (uint32_t i = 0; i < init_size; ++i) {
        _hash_nodemgr_init(&table->node_mgr[i]);
    }

    return table;
}

static
void _hash_tbl_delete(_fhash* table)
{
    if (!table) return;

    uint32_t size = table->index_size;

    for (uint32_t i = 0; i < size; ++i) {
        _fhash_node_mgr* mgr = &table->node_mgr[i];
        _hash_nodemgr_destroy(mgr);
    }

    free(table);
}

static
void _hash_tbl_set(_fhash* table, fhash_opt* opt,
               const void* key, key_sz_t key_sz,
               const void* value, value_sz_t value_sz)
{
    // if reach to the max size, the program should exit ASAP
    // technically, it cannot be happened
    assert(table->slots_used != SIZE_MAX);

    uint32_t idx = _hash_tbl_calculate_idx(table, opt, key, key_sz);
    _fhash_node_mgr* mgr = &table->node_mgr[idx];

    _fhash_node* node = _hash_nodemgr_find(mgr, opt, key, key_sz, NODE_VALID);
    if (!node) {
        size_t old_used = _hash_nodemgr_size(mgr);
        _hash_nodemgr_add(mgr, key, key_sz, value, value_sz);
        table->slots_used++;

        if (old_used == 0) {
            table->index_used++;
        }
    } else {
        _hash_nodemgr_set(mgr, node, key, key_sz, value, value_sz);
    }
}

// only set value if the node exist
// return 1 if success
// return 0 when the node not exist
static
int _hash_tbl_set_only(_fhash* table, fhash_opt* opt,
               const void* key, key_sz_t key_sz,
               const void* value, value_sz_t value_sz)
{
    uint32_t idx = _hash_tbl_calculate_idx(table, opt, key, key_sz);
    _fhash_node_mgr* mgr = &table->node_mgr[idx];

    _fhash_node* node = _hash_nodemgr_find(mgr, opt, key, key_sz, NODE_VALID);
    if (!node) {
        return 0;
    } else {
        _hash_nodemgr_set(mgr, node, key, key_sz, value, value_sz);
        return 1;
    }
}

static
void* _hash_tbl_get(_fhash* table, fhash_opt* opt,
               const void* key, key_sz_t key_sz, value_sz_t* value_sz)
{
    uint32_t idx = _hash_tbl_calculate_idx(table, opt, key, key_sz);
    _fhash_node_mgr* mgr = &table->node_mgr[idx];

    _fhash_node* node = _hash_nodemgr_find(mgr, opt, key, key_sz, NODE_VALID);

    // try to update value_sz
    if (value_sz) {
        if (node) {
            *value_sz = _hash_node_valuesz(node);
        } else {
            *value_sz = 0;
        }
    }

    if (node) {
        return _hash_node_value(node);
    } else {
        return NULL;
    }
}

static
void _hash_tbl_del(_fhash* table, fhash_opt* opt,
               const void* key, key_sz_t key_sz)
{
    uint32_t idx = _hash_tbl_calculate_idx(table, opt, key, key_sz);
    _fhash_node_mgr* mgr = &table->node_mgr[idx];

    _fhash_node* node = _hash_nodemgr_find(mgr, opt, key, key_sz, NODE_VALID);
    if (!node) {
        return;
    }

    _hash_nodemgr_del(mgr, node);
    table->slots_used--;

    if (_hash_nodemgr_used(mgr) == 0) {
        table->index_used--;
    }
}

static
void* _hash_tbl_fetch_and_del(_fhash* table, fhash_opt* opt,
                         const void* key, key_sz_t key_sz,
                         void* value, value_sz_t value_sz)
{
    uint32_t idx = _hash_tbl_calculate_idx(table, opt, key, key_sz);
    _fhash_node_mgr* mgr = &table->node_mgr[idx];

    _fhash_node* node = _hash_nodemgr_find(mgr, opt, key, key_sz, NODE_VALID);
    if (!node) {
        return NULL;
    }

    // calculate copy size
    value_sz_t node_sz = _hash_node_valuesz(node);
    size_t copy_sz = (size_t)(node_sz > value_sz ? value_sz : node_sz);
    memcpy(value, _hash_node_value(node), copy_sz);

    _hash_nodemgr_del(mgr, node);
    table->slots_used--;

    if (_hash_nodemgr_used(mgr) == 0) {
        table->index_used--;
    }

    return value;
}

static
void* _hash_tbl_next(fhash_iter* iter)
{
    // 1. find the next valid node
    // 2. fill iterator's data and return node's value

    assert(iter);
    _fhash_node* node = NULL;
    _fhash*  table = iter->phash->current;
    uint32_t start = iter->index;

    // 1.
    if (!iter->key) {
        start = 0;
    }

    for (; start < table->index_size; start++) {
        _fhash_node_mgr* mgr = &table->node_mgr[start];
        node = _hash_nodemgr_next(mgr, iter, NODE_VALID);
        if (node) {
            break;
        }
    }

    // 2.
    if (!node) {
        return NULL;
    }

    iter->index = start;
    return iter->value;
}

//------------------------------------------------------------------------------
static
void _hash_rehash(fhash* phash, uint32_t new_size)
{
    assert(!phash->iter_refs);

    // 1. lock on rehashing mask
    // 2. migrate all key-value pairs from old table to new table
    // 3. swap current with temporary pointer then release the old table
    // 4. unlock rehashing mask

    // 1.
    phash->mask.rehashing = 1;

    // 2.
    phash->temporary = _hash_tbl_create(new_size);
    _fhash* new_table = phash->temporary;

    fhash_iter iter = fhash_iter_new(phash);
    void* data = NULL;
    while ((data = fhash_next(&iter))) {
        _hash_tbl_set(new_table, &phash->opt,
                      iter.key, iter.key_sz,
                      iter.value, iter.value_sz);
    }

    fhash_iter_release(&iter);

    // 3.
    _fhash* discarded_tbl = phash->current;
    phash->current = phash->temporary;
    phash->temporary = NULL;

    _hash_tbl_delete(discarded_tbl);

    // 4.
    phash->mask.rehashing = 0;
}

static
int _hash_need_rehash(fhash* phash)
{
    _fhash* table = phash->current;
    if (table->slots_used / table->index_size < AUTO_REHASH_THRESHOLD) {
        return 0;
    }

    return 1;
}

static
int _hash_can_rehash(fhash* phash)
{
    if (phash->iter_refs > 0) {
        return 0;
    }

    if (phash->mask.rehashing || phash->mask.performing) {
        return 0;
    }

    _fhash* table = phash->current;
    if (table->index_size == UINT32_MAX) {
        return 0;
    }

    return 1;
}

static
int _hash_try_rehash(fhash* phash)
{
    if (!_hash_can_rehash(phash)) {
        return 1;
    }

    if (_hash_need_rehash(phash)) {
        _fhash* table = phash->current;
        uint32_t new_size = FHASH_REHASH_SIZE(table);
        assert(new_size > 0);

        _hash_rehash(phash, new_size);
        return 0;
    }

    return 1;
}

static
void _hash_set_delay(fhash* phash, fhash_opt* opt,
                     const void* key, key_sz_t key_sz,
                     const void* value, value_sz_t value_sz)
{
    _fhash_node_mgr* mgr = &phash->delayed_actions;
    _fhash_node* node = _hash_nodemgr_find(mgr, opt, key, key_sz, NODE_VALID);
    if (!node) {
        _hash_nodemgr_add(mgr, key, key_sz, value, value_sz);
    } else {
        _hash_nodemgr_set(mgr, node, key, key_sz, value, value_sz);
    }
}

static
void _hash_perform_actions(fhash* phash)
{
    phash->mask.performing = 1;

    _fhash_node_mgr* actions = &phash->delayed_actions;
    size_t action_cnt = _hash_nodemgr_size(actions);
    if (action_cnt== 0) {
        return;
    }

    _fhash* table = phash->current;
    for (size_t i = 0; i < action_cnt; i++) {
        _fhash_node* node = &actions->node_list[i];
        if (!node->valid) {
            continue;
        }

        void* key       = _hash_node_key(node);
        key_sz_t key_sz = _hash_node_keysz(node);
        void* value     = _hash_node_value(node);
        value_sz_t value_sz = _hash_node_valuesz(node);

        _hash_tbl_set(table, &phash->opt, key, key_sz, value, value_sz);
        _hash_nodemgr_del(actions, node);
    }

    phash->mask.performing = 0;
    assert(_hash_nodemgr_used(actions) == 0);
}

//===================================OPEN API===================================
fhash* fhash_create(uint32_t init_size,
                    fhash_opt opt,
                    void* ud,
                    uint32_t flags)
{
    if( init_size == 0 ) {
        init_size = DEFAULT_TABLE_SIZE;
    }

    fhash* phash = calloc(1, sizeof(*phash));
    phash->ud = ud;
    phash->mask.value = flags;
    phash->opt = opt;
    phash->current = _hash_tbl_create(init_size);
    _hash_nodemgr_init(&phash->delayed_actions);

    if (!phash->opt.hash_alg) {
        phash->opt.hash_alg = _hash_fnv_algorithm;
    }

    return phash;
}

void fhash_delete(fhash* phash)
{
    if (!phash) return;

    _hash_tbl_delete(phash->current);
    _hash_tbl_delete(phash->temporary);
    free(phash);
}

void fhash_set(fhash* phash,
               const void* key, key_sz_t key_sz,
               const void* value, value_sz_t value_sz)
{
    assert(phash && key && key_sz > 0 && value && value_sz > 0);

    _fhash* table = phash->current;
    if (phash->iter_refs > 0) {
        int success = _hash_tbl_set_only(table, &phash->opt,
                                         key, key_sz, value, value_sz);
        if (!success) {
            _hash_set_delay(phash, &phash->opt, key, key_sz, value, value_sz);
        }
        return;
    }

    _hash_tbl_set(table, &phash->opt, key, key_sz, value, value_sz);

    if (phash->mask.auto_rehash) {
        _hash_try_rehash(phash);
    }
}

void* fhash_get(fhash* phash, const void* key, key_sz_t key_sz,
                value_sz_t* value_sz)
{
    assert(phash && key && key_sz > 0);

    _fhash* table = phash->current;
    return _hash_tbl_get(table, &phash->opt, key, key_sz, value_sz);
}

void fhash_del(fhash* phash, const void* key, key_sz_t key_sz)
{
    if (!phash || !key || key_sz <= 0) {
        return;
    }

    _fhash* table = phash->current;
    _hash_tbl_del(table, &phash->opt, key, key_sz);
}

void* fhash_fetch_and_del(fhash* phash,
                         const void* key, key_sz_t key_sz,
                         void* value, value_sz_t value_sz)
{
    if (!phash || !key || key_sz <= 0) {
        return NULL;;
    }

    if (!value || value_sz <= 0) {
        fhash_del(phash, key, key_sz);
        return NULL;
    } else {
        _fhash* table = phash->current;
        return _hash_tbl_fetch_and_del(table, &phash->opt,
                                       key, key_sz, value, value_sz);
    }
}

fhash_iter fhash_iter_new(fhash* phash)
{
    fhash_iter iter;
    memset(&iter, 0, sizeof(iter));

    iter.phash = phash;
    phash->iter_refs++;

    return iter;
}

void fhash_iter_release(fhash_iter* iter)
{
    if (!iter) {
        return;
    }

    fhash* phash = iter->phash;
    phash->iter_refs--;

    if (phash->iter_refs > 0) {
        return;
    }

    // try to perform the action list
    _hash_perform_actions(phash);

    // rehash if needed
    if (phash->mask.auto_rehash) {
        _hash_try_rehash(phash);
    }
}

void* fhash_next(fhash_iter* iter)
{
    return _hash_tbl_next(iter);
}

void fhash_foreach(fhash* phash, fhash_each_cb cb)
{
    fhash_iter iter = fhash_iter_new(phash);

    void* data = NULL;
    while ((data = fhash_next(&iter))) {
        if (cb(phash->ud, iter.key, iter.key_sz, iter.value, iter.value_sz)) {
            break;
        }
    }

    fhash_iter_release(&iter);
}

int fhash_rehash(fhash* phash, uint32_t new_size)
{
    assert(phash && !phash->temporary);

    if (!_hash_can_rehash(phash)) {
        return 1;
    }

    _fhash* table = phash->current;
    if (new_size == 0) {
        new_size = FHASH_REHASH_SIZE(table);
        assert(new_size > 0);
    }

    _hash_rehash(phash, new_size);
    return 0;
}

void fhash_profile(fhash* phash, fhash_profile_data* profile_data)
{
    _fhash* table = phash->current;
    size_t total_slots = 0;
    size_t used_slots = 0;

    for (uint32_t index = 0; index < table->index_size; index++) {
        _fhash_node_mgr* mgr = &table->node_mgr[index];

        size_t used = _hash_nodemgr_used(mgr);
        size_t size = _hash_nodemgr_size(mgr);
        if (used > 0) {
            printf("index: %u -- used: %zu, size: %zu, usage rate: %f\n",
                   index,
                   used,
                   size,
                   size > 0 ? (double)used / (double)size : 0.0);

            used_slots += used;
            total_slots += size;
        }
    }

    printf("[index]: used: %u, total: %u, usage rate: %f\n",
           table->index_used,
           table->index_size,
           table->index_size > 0
            ? (double)(table->index_used) / (double)(table->index_size)
            : 0.0);

    printf("[slots]: used: %zu, total: %zu, usage rate: %f\n",
           used_slots,
           total_slots,
           total_slots > 0
            ? (double)used_slots / (double)total_slots
            : 0.0);

    if (!profile_data) {
        return;
    }

    profile_data->index_used = table->index_used;
    profile_data->index_size = table->index_size;
    profile_data->used_slots = used_slots;
    profile_data->total_slots = total_slots;
}
