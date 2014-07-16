#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "common/compiler.h"
#include "fmbuf.h"

// mbuf, contain the two pointer: the header and tailer, let's see the basic
// use cases:
// 1. the mbuf is empty: when the header == tailer
// 2. the mbuf is full: when the next location of tailer == header

#define MBUF_START(pbuf)    ( (char*)(pbuf->buf) )
#define MBUF_END(pbuf)      ( pbuf->buf + pbuf->size )
#define MBUF_HEAD(pbuf)     ( pbuf->head )
#define MBUF_TAIL(pbuf)     ( pbuf->tail )
#define MBUF_SIZE(pbuf)     ( pbuf->size )
#define MBUF_USED(pbuf) \
    ( MBUF_HEAD(pbuf) <= MBUF_TAIL(pbuf) \
      ? MBUF_TAIL(pbuf) - MBUF_HEAD(pbuf) \
      : MBUF_SIZE(pbuf) - (MBUF_HEAD(pbuf) - MBUF_TAIL(pbuf)) + 1 )

#define MBUF_FREE(pbuf) \
    ( MBUF_HEAD(pbuf) <= MBUF_TAIL(pbuf) \
      ? MBUF_SIZE(pbuf) - (MBUF_TAIL(pbuf) - MBUF_HEAD(pbuf)) \
      : MBUF_HEAD(pbuf) - MBUF_TAIL(pbuf) - 1 )

#define MBUF_COPY(dst, src, sz) \
    if (sz > 0 && dst && src) { \
        memcpy(dst, src, sz); \
    }

struct _mbuf {
    size_t size;
    char*  head;
    char*  tail;
    char   buf[1];
};

fmbuf*    fmbuf_create(size_t size)
{
    fmbuf* pmbuf = malloc(sizeof(fmbuf) + size);
    pmbuf->size = size;
    pmbuf->head = pmbuf->tail = pmbuf->buf;

    return pmbuf;
}

fmbuf*    fmbuf_create1(size_t size, int fill)
{
    fmbuf* pmbuf = fmbuf_create(size);
    memset(pmbuf->buf, fill, size);

    return pmbuf;
}

void    fmbuf_delete(fmbuf* pbuf)
{
    free(pbuf);
}

int     fmbuf_push(fmbuf* pbuf, const void* data, size_t size)
{
    if (data && size > 0 && (size_t)MBUF_FREE(pbuf) >= size) {
        size_t tail_free = fmbuf_tail_free(pbuf);

        if (tail_free >= size) {
            memcpy(MBUF_TAIL(pbuf), data, size);
            MBUF_TAIL(pbuf) += size;

            if(MBUF_TAIL(pbuf) > MBUF_END(pbuf))
                MBUF_TAIL(pbuf) = MBUF_START(pbuf);
        } else {
            memcpy(MBUF_TAIL(pbuf), data, tail_free);
            size_t left = size - tail_free;
            memcpy(MBUF_START(pbuf), data + tail_free, left);
            MBUF_TAIL(pbuf) = MBUF_START(pbuf) + left;
        }

        return 0;    //push sucess
    }

    return 1;        //push failed
}

//pop data from head
//if head to end space is less than size then go on to pop from start
int        fmbuf_pop(fmbuf* pbuf, void* data, size_t size)
{
    if (size > 0 && size <= (size_t)MBUF_USED(pbuf)) {
        size_t tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

        if (tail_use >= size) {
            MBUF_COPY(data, MBUF_HEAD(pbuf), size);
            MBUF_HEAD(pbuf) += size;

            if (MBUF_HEAD(pbuf) > MBUF_END(pbuf))
                MBUF_HEAD(pbuf) = MBUF_START(pbuf);
        } else {
            MBUF_COPY(data, MBUF_HEAD(pbuf), tail_use);
            size_t left = size - tail_use;
            MBUF_COPY(data + tail_use, MBUF_START(pbuf), left);
            MBUF_HEAD(pbuf) = MBUF_START(pbuf) + left;
        }

        return 0;    //pop sucess
    }

    return 1;        //pop failed
}

// ensure you use the return value , that's safe
void*    fmbuf_vpop(fmbuf* pbuf, void* data, size_t size)
{
    if (data && size > 0 && size <= (size_t)MBUF_USED(pbuf)) {
        size_t tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

        if (tail_use >= size) {
            data = MBUF_HEAD(pbuf);
            MBUF_HEAD(pbuf) += size;

            if(MBUF_HEAD(pbuf) > MBUF_END(pbuf))
                MBUF_HEAD(pbuf) = MBUF_START(pbuf);
        } else {
            memcpy((char*)data, MBUF_HEAD(pbuf), tail_use);
            size_t left = size - tail_use;
            memcpy((char*)data + tail_use, MBUF_START(pbuf), left);
            MBUF_HEAD(pbuf) = MBUF_START(pbuf) + left;
        }

        return data;    //pop sucess
    }

    return NULL;        //pop failed
}

void*   fmbuf_rawget(fmbuf* pbuf, void* data, size_t size)
{
    if (data && size > 0 && size <= (size_t)MBUF_USED(pbuf)) {
        size_t tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

        if (tail_use >= size) {
            data = MBUF_HEAD(pbuf);
        } else {
            memcpy(data, MBUF_HEAD(pbuf), tail_use);
            size_t left = size - tail_use;
            memcpy(data + tail_use, MBUF_START(pbuf), left);
        }

        return data;    //get sucess
    }

    return    NULL;        //get failed
}

void*   fmbuf_alloc(fmbuf* pbuf, size_t size)
{
    if (fmbuf_tail_free(pbuf) < size)
        fmbuf_rewind(pbuf);

    if (fmbuf_tail_free(pbuf) >= size) {
        char* ptr = MBUF_TAIL(pbuf);
        MBUF_TAIL(pbuf) += size;

        return ptr;
    }

    return NULL;
}

void    fmbuf_rewind(fmbuf* pbuf)
{
    size_t head_free = fmbuf_head_free(pbuf);
    if (head_free == 0)
        return;

    memmove(MBUF_START(pbuf), MBUF_HEAD(pbuf), MBUF_USED(pbuf));

    fmbuf_head_seek(pbuf, head_free, FMBUF_SEEK_LEFT);
    fmbuf_tail_seek(pbuf, head_free, FMBUF_SEEK_LEFT);
}

void    fmbuf_clear(fmbuf* pbuf)
{
    MBUF_HEAD(pbuf) = MBUF_TAIL(pbuf) = MBUF_START(pbuf);
}

void    fmbuf_head_seek(fmbuf* pbuf, size_t offset, int direction)
{
    if (direction == FMBUF_SEEK_LEFT) {
        MBUF_HEAD(pbuf) -= offset;
    } else {
        MBUF_HEAD(pbuf) += offset;
    }
}

void    fmbuf_tail_seek(fmbuf* pbuf, size_t offset, int direction)
{
    if (direction == FMBUF_SEEK_LEFT) {
        MBUF_TAIL(pbuf) -= offset;
    } else {
        MBUF_TAIL(pbuf) += offset;
    }
}

size_t  fmbuf_used(fmbuf* pbuf)
{
    return MBUF_USED(pbuf);
}

size_t  fmbuf_free(fmbuf* pbuf)
{
    return MBUF_FREE(pbuf);
}

size_t  fmbuf_tail_free(fmbuf* pbuf)
{
    return MBUF_END(pbuf) - MBUF_TAIL(pbuf);
}

size_t  fmbuf_head_free(fmbuf* pbuf)
{
    return MBUF_HEAD(pbuf) - MBUF_START(pbuf);
}

static
fmbuf*  _increase_buf(fmbuf* pbuf, size_t size)
{
    size_t head_offset = MBUF_HEAD(pbuf) - MBUF_START(pbuf);
    size_t tail_offset = MBUF_TAIL(pbuf) - MBUF_START(pbuf);

    if (MBUF_HEAD(pbuf) <= MBUF_TAIL(pbuf)) {
        fmbuf* new_buf = realloc(pbuf, sizeof(fmbuf) + size);
        MBUF_HEAD(new_buf) = MBUF_START(new_buf) + head_offset;
        MBUF_TAIL(new_buf) = MBUF_START(new_buf) + tail_offset;
        MBUF_SIZE(new_buf) = size;
        return new_buf;
    } else {
        // 1. calculate used buffer size for:
        //   * start to tailer
        //   * header to end
        // 2. move the min part

        // 1.
        size_t increased_sz = size - MBUF_SIZE(pbuf);
        size_t left_used = tail_offset;
        size_t right_used = MBUF_USED(pbuf) - left_used;
        fmbuf* new_buf = realloc(pbuf, sizeof(fmbuf) + size);
        MBUF_HEAD(new_buf) = MBUF_START(new_buf) + head_offset;
        MBUF_TAIL(new_buf) = MBUF_START(new_buf) + tail_offset;

        // 2.
        if (left_used <= right_used && left_used <= increased_sz) {
            void* dst = MBUF_END(new_buf) + 1;
            memmove(dst, MBUF_START(new_buf), left_used);
            MBUF_TAIL(new_buf) = dst + left_used;
            MBUF_SIZE(new_buf) = size;

            if(MBUF_TAIL(new_buf) > MBUF_END(new_buf))
                MBUF_TAIL(new_buf) = MBUF_START(new_buf);
        } else {
            void* dst = MBUF_HEAD(new_buf) + increased_sz;
            memmove(dst, MBUF_HEAD(new_buf), right_used);
            MBUF_HEAD(new_buf) += increased_sz;
        }

        return new_buf;
    }
}

static
fmbuf* _decrease_buf(fmbuf* pbuf, size_t size)
{
    size_t tail_offset = MBUF_TAIL(pbuf) - MBUF_START(pbuf);

    if (MBUF_HEAD(pbuf) <= MBUF_TAIL(pbuf)) {
        if (tail_offset > size) {
            fmbuf_rewind(pbuf);
            tail_offset = MBUF_TAIL(pbuf) - MBUF_START(pbuf);
            size = tail_offset > size ? tail_offset : size;
        }

        size_t head_offset = MBUF_HEAD(pbuf) - MBUF_START(pbuf);
        fmbuf* new_buf = realloc(pbuf, sizeof(fmbuf) + size);
        MBUF_HEAD(new_buf) = MBUF_START(new_buf) + head_offset;
        MBUF_TAIL(new_buf) = MBUF_START(new_buf) + tail_offset;
        MBUF_SIZE(new_buf) = size;

        return new_buf;
    } else {
        size_t left_used = tail_offset;
        size_t right_used = MBUF_USED(pbuf) - left_used;

        size_t decreased_sz = MBUF_SIZE(pbuf) - size;
        size_t free_sz = MBUF_FREE(pbuf);
        size_t move_sz = free_sz >= decreased_sz ? decreased_sz : free_sz;
        size_t actual_sz = MBUF_SIZE(pbuf) - move_sz;
        size_t head_offset = MBUF_HEAD(pbuf) - MBUF_START(pbuf);

        void* dst = MBUF_HEAD(pbuf) - move_sz;
        memmove(dst, MBUF_HEAD(pbuf), right_used);
        MBUF_HEAD(pbuf) -= move_sz;
        head_offset = MBUF_HEAD(pbuf) - MBUF_START(pbuf);

        fmbuf* new_buf = realloc(pbuf, sizeof(fmbuf) + actual_sz);
        MBUF_HEAD(new_buf) = MBUF_START(new_buf) + head_offset;
        MBUF_TAIL(new_buf) = MBUF_START(new_buf) + tail_offset;
        MBUF_SIZE(new_buf) = actual_sz;

        return new_buf;
    }
}

fmbuf*  fmbuf_realloc(fmbuf* pbuf, size_t size)
{
    size_t total_size = MBUF_SIZE(pbuf);
    if (total_size == size) {
        return pbuf;
    } else if (total_size < size) {
        return _increase_buf(pbuf, size);
    } else { // total_size > size
        return _decrease_buf(pbuf, size);
    }
}

void*   fmbuf_head(fmbuf* pbuf)
{
    return MBUF_HEAD(pbuf);
}

void*   fmbuf_tail(fmbuf* pbuf)
{
    return MBUF_TAIL(pbuf);
}

size_t  fmbuf_size(fmbuf* pbuf)
{
    return MBUF_SIZE(pbuf);
}
