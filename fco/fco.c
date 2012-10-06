#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <stdint.h>

#include "fco.h"

#define FCO_DEFAULT_STACK_SIZE (1024*1024)

struct _fco {
    ucontext_t   prev_ctx;
    ucontext_t   ctx;
    fco_sched*   owner;
    fco_sched*   container;
    pfunc_co     pf;
    struct _fco* prev;
    struct _fco* next;
    int          status;
    char         stack[0];
};

struct _fco_sched {
    uint64_t     numco;
    fco*         current;
    void*        arg;
    struct _fco* head;
    struct _fco* tail;
};

fco_sched* fco_scheduler_create()
{
    fco_sched* sched = malloc(sizeof(fco_sched));
    if ( !sched ) return NULL;

    memset(sched, 0, sizeof(fco_sched));
    return sched;
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
fco* _fco_create(fco_sched* owner, pfunc_co pf)
{
    fco* co = malloc(sizeof(fco) + FCO_DEFAULT_STACK_SIZE);
    if ( !co ) return NULL;

    memset(co, 0, sizeof(fco) + FCO_DEFAULT_STACK_SIZE);
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
    } else if ( !co->next ) { // node at middle
        co->owner->tail = co->prev;
        if ( co->prev ) co->owner->tail->next = NULL;
    } else { // node at tail
        co->prev->next = co->next;
        co->next->prev = co->prev;
    }

    co->owner->numco--;
    if ( !co->owner->numco ) {
        co->owner->head = co->owner->tail = NULL;
    }

    free(co);
}

fco* fco_main_create(fco_sched* owner, pfunc_co pf)
{
    if ( !owner || !pf) return NULL;
    return _fco_create(owner, pf);
}

fco* fco_create(fco* co, pfunc_co pf)
{
    if ( !co || !co->owner || !pf ) return NULL;

    fco_sched* container = fco_scheduler_create();
    if ( !container ) return NULL;

    fco* subco = _fco_create(container, pf);
    if ( !subco ) {
        fco_scheduler_destroy(container);
        return NULL;
    }

    co->container = container;
    return subco;
}

static
void co_main(uint32_t co_low32, uint32_t co_hi32, uint32_t arg_low32, uint32_t arg_hi32)
{
    long long_co = (long)co_low32 | ((long)co_hi32 << 32);
    fco* co = (fco*)long_co;
    void* arg = co->owner->arg;

    void* ret = co->pf(co, arg);
    co->owner->arg = ret;
    co->status = FCO_STATUS_DEAD;
}

void* fco_resume(fco* co, void* arg)
{
    if ( !co ) return NULL;

    switch ( co->status ) {
        case FCO_STATUS_SUSPEND:
            co->status = FCO_STATUS_RUNNING;
            co->owner->arg = arg;
            swapcontext(&co->prev_ctx, &co->ctx);
            break;
        case FCO_STATUS_READY:
            getcontext(&co->ctx);
            co->ctx.uc_stack.ss_sp = co->stack;
            co->ctx.uc_stack.ss_size = FCO_DEFAULT_STACK_SIZE;
            co->ctx.uc_link = &co->prev_ctx;
            co->owner->current = co;
            co->owner->arg = arg;
            co->status = FCO_STATUS_RUNNING;
            long lco = (long)co;
            makecontext(&co->ctx, (void (*)(void)) co_main, 2, (uint32_t)lco,
                        (uint32_t)(lco >> 32));
            swapcontext(&co->prev_ctx, &co->ctx);
            break;
        default:
            break;
    }

    void* ret = co->owner->arg;
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
    swapcontext(&co->ctx, &co->prev_ctx);
    return co->owner->arg;
}

int fco_status(fco* co)
{
    if ( !co ) return FCO_STATUS_DEAD;
    return co->status;
}
