#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "compiler.h"
#include "fmbuf.h"

#define MBUF_START(pbuf)    ( (char*)(pbuf->buf) )
#define MBUF_END(pbuf)      ( pbuf->buf + pbuf->size - 1 )
#define MBUF_HEAD(pbuf)     ( pbuf->head )
#define MBUF_TAIL(pbuf)     ( pbuf->tail )
#define MBUF_SIZE(pbuf)     ( pbuf->size )
#define MBUF_USED(pbuf)     ( MBUF_HEAD(pbuf) <= MBUF_TAIL(pbuf) ? MBUF_TAIL(pbuf) - MBUF_HEAD(pbuf) : MBUF_SIZE(pbuf) + MBUF_TAIL(pbuf) - MBUF_HEAD(pbuf) )
#define MBUF_FREE(pbuf)     ( MBUF_HEAD(pbuf) <= MBUF_TAIL(pbuf) ? MBUF_SIZE(pbuf) + MBUF_HEAD(pbuf) - MBUF_TAIL(pbuf) : MBUF_HEAD(pbuf) - MBUF_TAIL(pbuf) )
#define MBUF_FIX_ADDR(size) ( size + ((size & 3) > 0)*(4 - (size & 3))  )        //fix the addr 

struct _mbuf {
    size_t size;
    char*  head;
    char*  tail;
    char   buf[1];
};

fmbuf*    fmbuf_create(size_t size)
{
    if ( size > 0 ) {
        fmbuf* pmbuf = malloc(sizeof(fmbuf) + size);

        if ( !pmbuf ) return NULL;
        pmbuf->size = size;
        pmbuf->head = pmbuf->tail = pmbuf->buf;

        return pmbuf;
    }
    return NULL;
}

fmbuf*    fmbuf_create1(size_t size, int fill)
{
    if ( size > 0 ) {
        fmbuf* pmbuf = malloc(sizeof(fmbuf) + size);

        if ( !pmbuf ) return NULL;
        pmbuf->size = size;
        pmbuf->head = pmbuf->tail = pmbuf->buf;
        memset(pmbuf->buf, fill, size);

        return pmbuf;
    }
    return NULL;
}

void    fmbuf_delete(fmbuf* pbuf)
{
    free(pbuf);
}

int     fmbuf_push( fmbuf* pbuf, const void* data, size_t size)
{
    if ( pbuf && data && size > 0 && ((size_t)MBUF_FREE(pbuf) -1) >= size ) {
        size_t tail_free = MBUF_END(pbuf) - MBUF_TAIL(pbuf) + 1;

        if ( tail_free >= size ) {
            memcpy(MBUF_TAIL(pbuf), (char*)data, size);
            MBUF_TAIL(pbuf) += size;

            if( MBUF_TAIL(pbuf) > MBUF_END(pbuf) )
                MBUF_TAIL(pbuf) = MBUF_START(pbuf);
        } else {
            memcpy(MBUF_TAIL(pbuf), (char*)data, tail_free);
            size_t left = size - tail_free;
            memcpy(MBUF_START(pbuf), (char*)data+tail_free, left);
            MBUF_TAIL(pbuf) = MBUF_START(pbuf) + left;
        }

        return 0;    //push sucess
    }

    return 1;        //push failed
}

//pop data from head
//if head to end space is less than size then go on to pop from start
int        fmbuf_pop( fmbuf* pbuf, void* data, size_t size)
{
    if ( pbuf && data && size > 0 && size <= (size_t)MBUF_USED(pbuf) ) {
        size_t tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

        if ( tail_use >= size ) {
            memcpy((char*)data, MBUF_HEAD(pbuf), size);
            MBUF_HEAD(pbuf) += size;

            if ( MBUF_HEAD(pbuf) > MBUF_END(pbuf) )
                MBUF_HEAD(pbuf) = MBUF_START(pbuf);
        } else {
            memcpy((char*)data, MBUF_HEAD(pbuf), tail_use);
            size_t left = size - tail_use;
            memcpy((char*)data+tail_use, MBUF_START(pbuf), left);
            MBUF_HEAD(pbuf) = MBUF_START(pbuf) + left;
        }

        return 0;    //pop sucess
    }

    return 1;        //pop failed
}

// ensure you use the return value , that's safe
void*    fmbuf_vpop(fmbuf* pbuf, void* data, size_t size)
{
    if ( pbuf && data && size > 0 && size <= (size_t)MBUF_USED(pbuf) ) {
        size_t tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

        if ( tail_use >= size ) {
            data = MBUF_HEAD(pbuf);
            MBUF_HEAD(pbuf) += size;

            if( MBUF_HEAD(pbuf) > MBUF_END(pbuf) )
                MBUF_HEAD(pbuf) = MBUF_START(pbuf);
        } else {
            memcpy((char*)data, MBUF_HEAD(pbuf), tail_use);
            size_t left = size - tail_use;
            memcpy((char*)data+tail_use, MBUF_START(pbuf), left);
            MBUF_HEAD(pbuf) = MBUF_START(pbuf) + left;
        }

        return data;    //pop sucess
    }

    return NULL;        //pop failed
}

// only use to move the head ptr forward tail for sometime optimizating performance
void    fmbuf_head_move(fmbuf* pbuf, size_t size)
{
    if ( pbuf && size > 0 && size <= (size_t)MBUF_USED(pbuf) ) {
        size_t tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

        if ( tail_use >= size ) {
            MBUF_HEAD(pbuf) += size;

            if( unlikely(MBUF_HEAD(pbuf) > MBUF_END(pbuf)) )
                MBUF_HEAD(pbuf) = MBUF_START(pbuf);
        } else {
            size_t left = size - tail_use;
            MBUF_HEAD(pbuf) = MBUF_START(pbuf) + left;
        }
    }
}

void*   fmbuf_getraw(fmbuf* pbuf, void* data, size_t size)
{
    if ( pbuf && data && size > 0 && size <= (size_t)MBUF_USED(pbuf) ) {
        size_t tail_use = MBUF_END(pbuf) - MBUF_HEAD(pbuf) + 1;

        if ( tail_use >= size ) {
            data = MBUF_HEAD(pbuf);
        } else {
            memcpy((char*)data, MBUF_HEAD(pbuf), tail_use);
            size_t left = size - tail_use;
            memcpy((char*)data+tail_use, MBUF_START(pbuf), left);
        }

        return data;    //get sucess
    }

    return    NULL;        //get failed
}

void*   fmbuf_alloc(fmbuf* pbuf, size_t size)
{
    if ( fmbuf_tail_free(pbuf) < size )
        fmbuf_rewind(pbuf);

    if ( fmbuf_tail_free(pbuf) >= size ) {
        char* ptr = MBUF_TAIL(pbuf);
        MBUF_TAIL(pbuf) += size;

        return ptr;
    }

    return NULL;
}

void    fmbuf_rewind(fmbuf* pbuf)
{
    int head_free = fmbuf_head_free(pbuf);

    if (head_free == 0)
        return;

    memmove(MBUF_START(pbuf), MBUF_HEAD(pbuf), MBUF_USED(pbuf));

    fmbuf_head_seek(pbuf, -head_free);
    fmbuf_tail_seek(pbuf, -head_free);
}

void    fmbuf_clear(fmbuf* pbuf)
{
    MBUF_HEAD(pbuf) = MBUF_TAIL(pbuf) = MBUF_START(pbuf);
}

void    fmbuf_head_seek(fmbuf* pbuf, int offset)
{
    MBUF_HEAD(pbuf) += offset;
}

void    fmbuf_tail_seek(fmbuf* pbuf, int offset)
{
    MBUF_TAIL(pbuf) += offset;
}

size_t  fmbuf_used(fmbuf* pbuf)
{
    return MBUF_USED(pbuf);
}

size_t  fmbuf_total_free(fmbuf* pbuf)
{
    return MBUF_FREE(pbuf);
}

size_t  fmbuf_free(fmbuf* pbuf)
{
    return fmbuf_total_free(pbuf) - 1;
}

size_t  fmbuf_tail_free(fmbuf* pbuf)
{
    return MBUF_END(pbuf) - MBUF_TAIL(pbuf) + 1;
}

size_t  fmbuf_head_free(fmbuf* pbuf)
{
    return MBUF_HEAD(pbuf) - MBUF_START(pbuf);
}

fmbuf*   fmbuf_realloc(fmbuf* pbuf, size_t size)
{
    size_t total_size = MBUF_SIZE(pbuf);
    if ( total_size == size )
        return pbuf;

    size_t tail_pos = MBUF_TAIL(pbuf) - MBUF_START(pbuf);
    if ( tail_pos > size ) {
        fmbuf_rewind(pbuf);
        tail_pos = MBUF_TAIL(pbuf) - MBUF_START(pbuf);
        size = tail_pos > size ? tail_pos : size;
    }

    size_t head_pos = MBUF_HEAD(pbuf) - MBUF_START(pbuf);
    fmbuf* new_buf = realloc(pbuf, sizeof(fmbuf) + size);

    MBUF_SIZE(new_buf) = size;
    MBUF_HEAD(new_buf) = MBUF_START(new_buf) + head_pos;
    MBUF_TAIL(new_buf) = MBUF_START(new_buf) + tail_pos;

    return new_buf;
}

void*   fmbuf_get_head(fmbuf* pbuf)
{
    return MBUF_HEAD(pbuf);
}

void*   fmbuf_get_tail(fmbuf* pbuf)
{
    return MBUF_TAIL(pbuf);
}

size_t  fmbuf_size(fmbuf* pbuf)
{
    return MBUF_SIZE(pbuf);
}
