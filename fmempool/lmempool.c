//base info: create by hyz
//effect: thread cache mempool


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include "lmempool.h"
#include "lmutex.h"

#ifdef _USE_MEM_GC_
#include "ltimer.h"
#endif

#undef malloc
#undef free
#undef realloc
#undef calloc

//#define STAT_SYS_ALLOC_SIZE

#define FMEM_CPU_CACHELINE  128
#define FMEM_HASH_TH_SIZE   20
#define FMEM_HASH_MGR_SIZE  20
#define FMEM_FREE_LIST_SIZE (1024 * 512)
#define FMEM_DEFAULT_COUNT  256
#define FMEM_GC_LIMIT       256
#define FMEM_GC_TIMEOUT     (5000000000l)
#define FMEM_GC_INTERVAL    (10000)
#define FMEM_STEAL_COUNT    128
#define FMEM_SYS_ALLOC_IDX  -1
#define FMEM_DOGC           1
#define FMEM_UNDOGC         0
#define FMEM_CREATE_FB      1
#define FMEM_NOT_CREATE_FB  0
#define FMEM_CREATE_TR      1
#define FMEM_NOT_CREATE_TR  0

#define FMEM_PAGE_SIZE      (1024 * 4)
#define FMEM_PAGE_SIZE2     (1024 * 8)
#define FMEM_PAGE_SIZE4     (1024 * 16)
#define FMEM_PAGE_SIZE8     (1024 * 32)    //max size

static const size_t b_size[] = {
    8, 16, 24, 32, 48, 64, 128, 
    256, 512, 768, 1024, 2048, 
    FMEM_PAGE_SIZE,
    FMEM_PAGE_SIZE2,
    FMEM_PAGE_SIZE4,
    FMEM_PAGE_SIZE8                // max size 32K
};

#define FMEM_BLOCK_TYPE_SIZE (int)(sizeof(b_size) / sizeof(size_t))

#pragma pack(4)
typedef struct {
    int idx:8;
    unsigned int alloc_len:24;        // for optimization realloc
} fb_head;

typedef struct _fb {
    fb_head b_head;

    union {
        char        block[0];
        struct _fb* next;
    }block_data;
} free_block;

typedef struct _sys_fb {
    size_t     len;  //use for stat
    free_block fb;
} sys_block;
#pragma pack()

typedef struct {
    size_t      block_size;
    size_t      count;
    int         gc_measure;
    free_block* head;
} free_list;

typedef struct {
    free_block* fb_head;
    free_block* fb_tail;
} fcut_list;

typedef unsigned char byte;

typedef struct {
    pthread_t  tid;
    free_list* pth_mgr[FMEM_BLOCK_TYPE_SIZE];
    byte*      pmap;    // split index map for cpu cache pseudo sharing

#ifdef _USE_MEM_GC_
    f_timer    ti;      // gc timer
#endif
} fl_mgr;

typedef struct {
    fl_mgr*  pcenter_mgr;   // center free list

#ifdef STAT_SYS_ALLOC_SIZE
    size_t   sys_alloc_size; // over 32K alloc from sys
    spin_var lock;
#endif

    spin_var center_lock[FMEM_BLOCK_TYPE_SIZE];
    pthread_key_t key;
} fmem_pool;

// global memory pool
fmem_pool* volatile pmem = NULL;
static pthread_once_t init_create = PTHREAD_ONCE_INIT;

static inline
free_block* _f_pop(free_list* fl);
static inline
void        _f_push(free_list* fl, free_block* block);

static inline
void*   f_malloc(size_t size)
{
    return malloc(size + FMEM_CPU_CACHELINE);
}

// for test
static inline
int     f_count(free_list* fl)
{
    int i = 0;
    free_block* fb = fl->head;
    while(fb) {
        ++i;
        fb = fb->block_data.next;
    }

    return i;
}

static inline
int     _f_bsearch(size_t size, int start, int end)
{
    if ( start == end ) return start;
    int half = start + ((end - start) >> 1);

    if ( b_size[half] > size )
        return _f_bsearch(size, start, half);
    else if ( b_size[half] == size )
        return half;
    else
        return _f_bsearch(size, half + 1, end);
}

static inline
int     f_bsearch(size_t size)
{
    return _f_bsearch(size, 0, FMEM_BLOCK_TYPE_SIZE-1);
}

static inline
int     f_findidx(fl_mgr* mgr, size_t size)
{
    if( size > FMEM_PAGE_SIZE8 ) return -1;
    return mgr->pmap[(int)size - 1];
}

static
byte*   f_create_map()
{
    byte* pmap = f_malloc(sizeof(byte) * FMEM_PAGE_SIZE8);

    int i, j, k = 0, start = 0;
    for (i=0; i<FMEM_BLOCK_TYPE_SIZE; ++i){
        for ( j=start; j<(int)b_size[i]; ++j ){
            pmap[k++] = i;
        }
        start = b_size[i];
    }

    return pmap;
}

static
free_block* f_block_create(int index, size_t fb_size, size_t total_size)
{
    char* fb = f_malloc(total_size);
    char* iter = fb;
    char* end = fb + total_size;
    char* fb_next;

    int i=0;
    for( ; iter<end; iter += fb_size ){
        ((free_block*)iter)->b_head.idx = index;
        ((free_block*)iter)->b_head.alloc_len = 0;

        fb_next = iter + fb_size;
        if ( fb_next < end )
            ((free_block*)iter)->block_data.next = (free_block*)fb_next;
        else
            ((free_block*)iter)->block_data.next = NULL;

        i++;
    }

    return (free_block*)fb;
}

static
free_list* fl_create(int index, size_t len, int is_create_fb)
{
    size_t real_fb_size = sizeof(fb_head) + len;

    free_list* fl = f_malloc(sizeof(free_list));
    fl->block_size = real_fb_size;
    fl->count = 0;
    fl->gc_measure = 0;
    fl->head = NULL;

    if ( is_create_fb ) {
        size_t count = FMEM_DEFAULT_COUNT;
        free_block* fb = f_block_create(index, real_fb_size, real_fb_size * count);
        fl->head = fb;
        fl->count = count;
    }

    return fl;
}

// gc proc
void    f_gc(void* arg)
{
    //printf("gc run\n");
    fl_mgr* pmgr = (fl_mgr*)arg;

    int i;
    for (i=0; i<FMEM_BLOCK_TYPE_SIZE; ++i){
        free_list* fl = pmgr->pth_mgr[i];

        if( fl->count > FMEM_GC_LIMIT )
            fl->gc_measure = fl->count >> 1;
        else
            fl->gc_measure = 0;

        //printf("size=%lu count=%lu gc_measure = %d\n", fl->block_size, fl->count, fl->gc_measure);
    }
}

static
fl_mgr* flmgr_create(int is_create_fb, int is_create_timer)
{
    fl_mgr* mgr = f_malloc(sizeof(fl_mgr));

    int i;
    for ( i=0; i<FMEM_BLOCK_TYPE_SIZE; ++i ) {
        free_list* fl = fl_create(i, b_size[i], is_create_fb);
        mgr->pth_mgr[i] = fl;
    }

    mgr->pmap = f_create_map();
    mgr->tid = pthread_self();

#ifdef _USE_MEM_GC_
    if ( is_create_timer ) {
        ftimer_create(&mgr->ti, FMEM_GC_TIMEOUT, FMEM_GC_TIMEOUT, f_gc, mgr);
        ftimer_start(&mgr->ti);
    }
#endif

    return mgr;
}

void    fth_delete(void* data)
{
    fl_mgr* fl = (fl_mgr*)data;
    if ( !fl ) return;
    if ( !pthread_equal(fl->tid, pthread_self()) ) {
        printf("thread(%lu) has quit , do clean but the arg has error\n", pthread_self());
        return;
    }

#ifdef _USE_MEM_GC_
    ftimer_del(&fl->ti);
#endif
    free(fl->pmap);

    int i;
    for ( i=0; i<FMEM_BLOCK_TYPE_SIZE; ++i ) {
        // move the all block into center
        free_block* fb = NULL;
        while( (fb = _f_pop(fl->pth_mgr[i])) ){
            spin_lock(&pmem->center_lock[i]);
            _f_push(pmem->pcenter_mgr->pth_mgr[i], fb);
            spin_unlock(&pmem->center_lock[i]);
        }

        free(fl->pth_mgr[i]);
    }
    free(fl);
}

static inline
void    f_mp_create()
{
    fmem_pool* pmem_t = f_malloc(sizeof(fmem_pool));
    pmem_t->pcenter_mgr = flmgr_create(FMEM_CREATE_FB, FMEM_NOT_CREATE_TR);

#ifdef STAT_SYS_ALLOC_SIZE
    pmem_t->sys_alloc_size = 0;
    spin_init(&pmem_t->lock);
#endif
    int i;
    for (i=0; i<FMEM_BLOCK_TYPE_SIZE; ++i)
        spin_init(&pmem_t->center_lock[i]);

    pthread_key_create(&pmem_t->key, fth_delete);
    pmem = pmem_t;

    printf("mempool init complete tid=%lu\n", pthread_self());
#ifdef _USE_MEM_GC_
    printf("mempool gc on\n");
#endif
}

static inline
void    f_cut_list(free_list* fl, fcut_list* fc, int count)
{
    free_block* head = fl->head;
    free_block* tail = head;

    int i = 0;
    if ( head ) {
        fl->count -= 1;
        for (i=1; i<count && tail->block_data.next; ++i, fl->count -= 1)
            tail = tail->block_data.next;

        fl->head = tail->block_data.next;
        tail->block_data.next = NULL;
    }

    fc->fb_head = head;
    fc->fb_tail = tail;
}

static
void    f_steal_from_center(free_list* fl, int idx)
{
    free_list* c_fl = pmem->pcenter_mgr->pth_mgr[idx];

    spin_lock(&pmem->center_lock[idx]);
    if ( fl->head ) {
        spin_unlock(&pmem->center_lock[idx]);
        return;
    }

    free_block* head = c_fl->head;
    int i = 0;
    if ( head ) {
        free_block* tail = c_fl->head;
        c_fl->count -= 1;
        for (i=1;
             i<FMEM_STEAL_COUNT && tail->block_data.next; 
             ++i, c_fl->count -= 1) {
            tail = tail->block_data.next;
        }

        c_fl->head = tail->block_data.next;
        tail->block_data.next = NULL;
    }

    spin_unlock(&pmem->center_lock[idx]);

    fl->head = head;
    fl->count += i;
}

static inline
void    _f_do_gc(free_list* fl, int idx)
{
    fcut_list fc;
    int move_count = fl->gc_measure;

    f_cut_list(fl, &fc, move_count);
    free_list* c_fl = pmem->pcenter_mgr->pth_mgr[idx];
    spin_lock(&pmem->center_lock[idx]);

    free_block* next = c_fl->head;
    c_fl->head = fc.fb_head;
    fc.fb_tail->block_data.next = next;
    c_fl->count += move_count;

    spin_unlock(&pmem->center_lock[idx]);
}

static inline
void    f_do_gc(free_list* fl, int idx)
{
    if ( fl->gc_measure ) {
        _f_do_gc(fl, idx);
        fl->gc_measure = 0;
    }
}

static inline
free_block* _f_pop(free_list* fl)
{
    free_block* fb = fl->head;
    if ( fb ){
        fl->head = fl->head->block_data.next;
        fl->count -= 1;
    }

    return fb;
}

static inline
free_block*    f_pop(free_list* fl, int idx)
{
    free_block* fb = fl->head;

    if ( !fb ) {
        f_steal_from_center(fl, idx);

        fb = fl->head;
        if ( !fb ) {
            size_t fb_size = fl->block_size;
            size_t count = FMEM_DEFAULT_COUNT;

            fl->head = f_block_create(idx, fb_size, fb_size * count);
            fl->count = count;
            fb = fl->head;
        }
    }

    return _f_pop(fl);
}

static inline
void    _f_push(free_list* fl, free_block* block)
{
    if ( !block) return;
    block->b_head.alloc_len = 0;    //reset 0 mean it's free

    if ( fl->head )
        block->block_data.next = fl->head;
    else
        block->block_data.next = NULL;

    fl->head = block;
    fl->count += 1;
}

static inline
void*   _f_alloc_fromsys(size_t size)
{
    sys_block* block = malloc(size + sizeof(size_t) + sizeof(fb_head));
    block->len = size;
    block->fb.b_head.idx = FMEM_SYS_ALLOC_IDX;

#ifdef STAT_SYS_ALLOC_SIZE
    spin_lock(&pmem->lock);
    pmem->sys_alloc_size += size;
    spin_unlock(&pmem->lock);
#endif

    return block->fb.block_data.block;
}

static inline
void    _f_free_tosys(void* data, size_t len)
{
#ifdef STAT_SYS_ALLOC_SIZE
    spin_lock(&pmem->lock);
    pmem->sys_alloc_size -= len;
    spin_unlock(&pmem->lock);
#endif

    free(data);
}

static inline
void    _f_free(void* ptr)
{
    free_block* fb = (free_block*)((fb_head*)ptr - 1);
    int idx = fb->b_head.idx;
    if( fb->b_head.alloc_len == 0 ){
        printf("fatal error:double free ptr\n");
        abort();
    }

    fl_mgr* flmgr = pthread_getspecific(pmem->key);
    if ( !flmgr ) {    // the thread unknown
        free_list* fl = pmem->pcenter_mgr->pth_mgr[idx];

        spin_lock(&pmem->center_lock[idx]);
        _f_push(fl, fb);
        spin_unlock(&pmem->center_lock[idx]);

        return;
    }

    free_list* fl = flmgr->pth_mgr[idx];
    _f_push(fl, fb);
    f_do_gc(fl, idx);
}

static inline
void*   _f_alloc(fl_mgr* pmgr, size_t size)
{
    if ( size > FMEM_PAGE_SIZE8 )
        return _f_alloc_fromsys(size);

    int idx = f_findidx(pmgr, size);
    free_list* fl = pmgr->pth_mgr[idx];
    free_block* block = f_pop(fl, idx);

    if ( block ) {
        block->b_head.alloc_len = size;
        return block->block_data.block;
    }

    return NULL;
}

inline
void*   f_alloc(size_t size)
{
    if ( size == 0 ) return NULL;
    pthread_once(&init_create, f_mp_create);

    fl_mgr* flmgr = pthread_getspecific(pmem->key);
    if ( !flmgr ) {
        // the thread cache get from center
        flmgr = flmgr_create(FMEM_NOT_CREATE_FB, FMEM_CREATE_TR);
        pthread_setspecific(pmem->key, flmgr);
    }

    return _f_alloc(flmgr, size);
}

inline
void    f_free(void* ptr)
{
    if ( !ptr ) return;
    fb_head* data = ((fb_head*)ptr - 1);
    int idx = data->idx;

    if ( idx == FMEM_SYS_ALLOC_IDX ) {
        void* sys_data = (size_t*)data - 1;
        size_t len = *((size_t*)sys_data);
        _f_free_tosys(sys_data, len);
        return;
    }

    _f_free(ptr);
}

void*   _f_realloc_bysys(void* sys_data, size_t size)
{
    sys_block* block = realloc(sys_data, size + sizeof(size_t) + sizeof(fb_head));
    block->len = size;
    block->fb.b_head.idx = FMEM_SYS_ALLOC_IDX;

    return block->fb.block_data.block;
}

static inline
void    f_memcpy(free_list* fl, int idx, void* des,
                    free_block* fb, size_t len,
                    int thread_safe)
{
    memcpy(des, fb->block_data.block, len);
    if ( thread_safe )
        _f_push(fl, fb);
    else {
        spin_lock(&pmem->center_lock[idx]);
        _f_push(fl, fb);
        spin_unlock(&pmem->center_lock[idx]);
    }
}

static inline
free_block* f_get_tail(free_list* fl)
{
    free_block* fb = fl->head;
    if ( !fb ) return fb;

    while (fb->block_data.next) {
        fb = fb->block_data.next;
    }

    return fb;
}

static
free_block* f_pop_from_center(free_list* fl, int idx, size_t size)
{
    spin_lock(&pmem->center_lock[idx]);

    free_block* fb = _f_pop(fl);
    if ( fb ) {
        spin_unlock(&pmem->center_lock[idx]);
        return fb;
    }

    size_t count = FMEM_STEAL_COUNT;
    size_t real_fb_size = sizeof(fb_head) + size;
    free_block* new_fb_list = f_block_create(idx, real_fb_size, real_fb_size * count);

    spin_lock(&pmem->center_lock[idx]);
    free_block* next = fl->head;
    fl->head = new_fb_list;
    free_block* tail = f_get_tail(fl);
    tail->block_data.next = next;
    fl->count += count;

    fb = _f_pop(fl);
    spin_unlock(&pmem->center_lock[idx]);

    return fb;
}

void*   _f_realloc(void* data, size_t size, int idx)
{
    free_block* fb = (free_block*)data;
    size_t old_len = fb->b_head.alloc_len;
    int thread_safe = 1;

    fl_mgr* flmgr = pthread_getspecific(pmem->key);
    if ( !flmgr ) {
        flmgr = pmem->pcenter_mgr;
        thread_safe = 0;
    }

    int new_idx = f_findidx(flmgr, size);
    if ( new_idx == idx ) {
        fb->b_head.alloc_len = size;
        return fb->block_data.block;
    }

    free_list* src_fl = flmgr->pth_mgr[idx];
    void* new_data = NULL;
    if ( size > FMEM_PAGE_SIZE8 )
        // new space need to alloc from sys
        new_data = _f_alloc_fromsys(size);
    else {
        free_list* des_fl = flmgr->pth_mgr[new_idx];
        free_block* new_fb = NULL;
        if ( thread_safe )
            // alloc from another index of freelist
            new_fb = f_pop(des_fl, new_idx);
        else
            new_fb = f_pop_from_center(des_fl, new_idx, size);

        new_fb->b_head.alloc_len = size;
        new_data = new_fb->block_data.block;
    }

    f_memcpy(src_fl, idx, new_data, fb, old_len, thread_safe);

    return new_data;
}

void*   f_realloc(void* ptr, size_t size)
{
    if ( !ptr ) return f_alloc(size);        // same as ANSI C

    fb_head* data = ((fb_head*)ptr - 1);
    int idx = data->idx;

    if ( idx == FMEM_SYS_ALLOC_IDX ){
        void* sys_data = (size_t*)data - 1;
        return _f_realloc_bysys(sys_data, size);
    }

    return _f_realloc(data, size, idx);
}

void*   f_calloc(size_t num, size_t size)
{
    if( !num || !size ) return NULL;

    void* data = f_alloc(size * num);
    if ( data )
        memset(data, 0, size * num);

    return data;
}
