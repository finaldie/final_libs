#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <stdint.h>
#include <errno.h>

#include "fco.h"

#define FCO_DEFAULT_STACK_SIZE (1024*1024)

typedef struct plugin_node {
    phook_cb cb;
    void*    arg;
    struct plugin_node* next;
} plugin_node;

typedef struct plugin_meta {
    plugin_node* before_chain_head;
    plugin_node* after_chain_head;
    ucontext_t   main_ctx;
} plugin_meta;

#pragma pack(4)
struct _fco {
    ucontext_t   ctx;
    ucontext_t*  prev_ctx;
    fco_sched*   root;
    fco_sched*   owner;
    fco_sched*   container;
    pfunc_co     pf;
    struct _fco* prev;
    struct _fco* next;
    int          status;
    char         stack[0];
};
#pragma pack()

struct _fco_sched {
    uint64_t       numco;
    volatile void* arg;
    struct _fco*   head;
    struct _fco*   tail;
    char           plugin_data[0];    // only root used
};

fco_sched* _fco_scheduler_create(int is_root)
{
    size_t tot_size = sizeof(fco_sched) + (is_root ? sizeof(plugin_meta) : 0);
    fco_sched* sched = malloc(tot_size);
    if ( !sched ) return NULL;

    memset(sched, 0, tot_size);
    return sched;
}

fco_sched* fco_scheduler_create()
{
    return _fco_scheduler_create(1);
}

void fco_scheduler_destroy(fco_sched* fco_sched)
{
    if ( !fco_sched ) return;

    while ( fco_sched->head ) {
        fco* node = fco_sched->head;
        if ( node->container ) {
            fco_scheduler_destroy(node->container);
        }

        fco_sched->head = node->next;
        free(node);
    }

    free(fco_sched);
}

static
plugin_node* _fco_create_plugin_node(phook_cb cb, void* arg)
{
    plugin_node* node = malloc(sizeof(plugin_node));
    if ( !node ) return NULL;

    node->cb = cb;
    node->arg = arg;
    node->next = NULL;
    return node;
}

void fco_register_plugin(fco_sched* root, void* arg, plugin_init init,
                               phook_cb before_sw, phook_cb after_sw)
{
    if ( !root ) return;
    plugin_meta* meta = (plugin_meta*)(root->plugin_data);
    plugin_node* anode = _fco_create_plugin_node(after_sw, arg);
    plugin_node* bnode = _fco_create_plugin_node(before_sw, arg);
    if ( !anode || !bnode ) {
        free(anode);
        free(bnode);
        return;
    }

    bnode->next = meta->before_chain_head;
    meta->before_chain_head = bnode;

    anode->next = meta->after_chain_head;
    meta->after_chain_head = anode;

    init(root, arg);
}

static
void __call_plugin(plugin_node* head, fco* co)
{
    plugin_node* node = head;
    while ( node ) {
        node->cb(co, node->arg);
        node = node->next;
    }
}

// type: 0 -- call before chain
// type: 1 -- call after chain
static
void _fco_call_plugin(fco* co, int type)
{
    plugin_meta* meta = (plugin_meta*)(co->root->plugin_data);
    if ( type ) {
        __call_plugin(meta->after_chain_head, co);
    } else {
        __call_plugin(meta->before_chain_head, co);
    }
}

static
fco* _fco_create(fco_sched* root, fco_sched* owner, ucontext_t* prev_ctx, pfunc_co pf)
{
    fco* co = malloc(sizeof(fco) + FCO_DEFAULT_STACK_SIZE);
    if ( !co ) return NULL;

    memset(co, 0, sizeof(fco) + FCO_DEFAULT_STACK_SIZE);
    co->prev_ctx = prev_ctx;
    co->root = root;
    co->owner = owner;
    co->pf = pf;
    co->status = FCO_STATUS_READY;
    co->owner->numco++;

    if ( co->owner->head == co->owner->tail &&
         co->owner->head == NULL ) {
        co->owner->head = co->owner->tail = co;
    } else {
       co->prev = co->owner->tail;
       co->owner->tail->next = co;
       co->owner->tail = co;
    }

    return co;
}

static
void _fco_delete(fco* co)
{
    if ( !co || !co->owner ) return;

    if ( !co->prev ) { // node at head
        co->owner->head = co->next;
        if ( co->next ) co->owner->head->prev = NULL;
    } else if ( !co->next ) { // node at tail
        co->owner->tail = co->prev;
        if ( co->prev ) co->owner->tail->next = NULL;
    } else { // node at middle
        co->prev->next = co->next;
        co->next->prev = co->prev;
    }

    co->owner->numco--;
    if ( !co->owner->numco ) {
        co->owner->head = co->owner->tail = NULL;
    }

    free(co);
}

fco* fco_main_create(fco_sched* root, pfunc_co pf)
{
    if ( !root || !pf) return NULL;
    plugin_meta* meta = (plugin_meta*)root->plugin_data;
    return _fco_create(root, root, &meta->main_ctx, pf);
}

fco* fco_create(fco* co, pfunc_co pf, int type)
{
    if ( !co || !co->owner || !pf ) return NULL;
    if ( (type < FCO_TYPE_ALONE) || (type > FCO_TYPE_CHILD) ) return NULL;

    if ( type == FCO_TYPE_ALONE ) {
        return _fco_create(co->root, co->root, &co->ctx, pf);
    } else {
        fco_sched* container = _fco_scheduler_create(0);
        if ( !container ) return NULL;

        fco* subco = _fco_create(co->root, container, &co->ctx, pf);
        if ( !subco ) {
            fco_scheduler_destroy(container);
            return NULL;
        }

        co->container = container;
        return subco;
    }
}

#if __WORDSIZE == 64
static
void co_main(uint32_t co_low32, uint32_t co_hi32)
{
    uintptr_t co_ptr = (uintptr_t)co_low32 | ((uintptr_t)co_hi32 << 32);
    fco* co = (fco*)co_ptr;
    void* arg = (void*)co->owner->arg;

    void* ret = co->pf(co, arg);
    co->owner->arg = ret;
    co->status = FCO_STATUS_DEAD;
}
#else
static
void co_main(fco* co)
{
    void* arg = (void*)co->owner->arg;

    void* ret = co->pf(co, arg);
    co->owner->arg = ret;
    co->status = FCO_STATUS_DEAD;
}
#endif

static
void _fco_do_swap(ucontext_t* save, ucontext_t* to)
{
    if ( swapcontext(save, to) ) {
        fprintf(stderr, "[FATAL] swapcontext failed, detail:%s\n", strerror(errno));
        exit(1);
    }
}

void* fco_resume(fco* co, void* arg)
{
    if ( !co ) return NULL;

    switch ( co->status ) {
        case FCO_STATUS_SUSPEND:
            co->status = FCO_STATUS_RUNNING;
            co->owner->arg = arg;
            _fco_call_plugin(co, 0);
            _fco_do_swap(co->prev_ctx, &co->ctx);
            _fco_call_plugin(co, 1);
            break;
        case FCO_STATUS_READY:
            if ( getcontext(&co->ctx) ) {
                fprintf(stderr, "[FATAL] getcontext failed, detail:%s\n", strerror(errno));
                exit(1);
            }

            co->ctx.uc_stack.ss_sp = co->stack;
            co->ctx.uc_stack.ss_size = FCO_DEFAULT_STACK_SIZE;
            co->ctx.uc_link = co->prev_ctx;
            co->owner->arg = arg;
            co->status = FCO_STATUS_RUNNING;
            uintptr_t lco = (uintptr_t)co;
#if __WORDSIZE == 64
            makecontext(&co->ctx, (void (*)(void)) co_main, 2, (uint32_t)lco,
                        (uint32_t)(lco >> 32));
#else
            makecontext(&co->ctx, (void (*)(void)) co_main, 1, lco);
#endif
            _fco_call_plugin(co, 0);
            _fco_do_swap(co->prev_ctx, &co->ctx);
            _fco_call_plugin(co, 1);
            break;
        default:
            fprintf(stderr, "[WARING] shouldn't resume a non-RUNNING or non-SUSPEND status co\n");
            return NULL;
    }

    void* ret = (void*)co->owner->arg;
    if ( co->status == FCO_STATUS_DEAD ) {
        if ( co->container ) {
            fco_scheduler_destroy(co->container);
        }
        _fco_delete(co);
    }

    return ret;
}

void* fco_yield(fco* co, void* arg)
{
    if ( !co ) return NULL;
    co->status = FCO_STATUS_SUSPEND;
    co->owner->arg = arg;
    _fco_call_plugin(co, 0);
    _fco_do_swap(&co->ctx, co->prev_ctx);
    _fco_call_plugin(co, 1);
    return (void*)co->owner->arg;
}

int fco_status(fco* co)
{
    if ( !co ) return FCO_STATUS_DEAD;
    return co->status;
}
