/*
 * =====================================================================================
 *
 *       Filename:  fnet_buff.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  11/18/2011 10:43:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  yuzhang hu
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "fmbuf/fmbuf.h"
#include "fev.h"
#include "fev_buff.h"

#define FEV_BUFF_DEFAULT_SIZE   (1024 * 4)

#pragma pack(4)
struct fev_buff {
    int             fd;
    fev_state*      fstate;
    fmbuf*          rbuf;
    fmbuf*          wbuf;
    fev_buff_read   read_cb;
    fev_buff_error  error_cb;
    void*           arg;
};
#pragma pack()

static
int fev_read(int fd, void* pbuf, size_t len)
{
    do {
        int read_size = read(fd, pbuf, len);

        if ( read_size == -1 ) {
            if( errno == EINTR )
                continue;
            else if( errno == EAGAIN )
                return 0;   // we can ignore this and simply to return 0
            return -1;
        } else if( read_size == 0 )
            return -1;
        else
            return read_size;
    } while (1);
}

static
int fev_write(int fd, const char* pbuf, size_t len)
{
    do {
        int bytes = write(fd, pbuf, len);

        if ( bytes > 0 )
            return bytes;
        else {
            if( errno == EINTR )
                continue;
            else if( errno == EAGAIN )
                return 0;
            else
                return -1;
        }
    } while (1);
}

static
void evbuff_read(fev_state* fev,
                 int fd     __attribute__((unused)),
                 int mask   __attribute__((unused)),
                 void* arg)
{
    fev_buff* evbuff = (fev_buff*)arg;
    if ( evbuff->read_cb )
        evbuff->read_cb(fev, evbuff, evbuff->arg);
}

static
void evbuff_write(fev_state* fev,
                  int fd    __attribute__((unused)),
                  int mask  __attribute__((unused)),
                  void* arg)
{
    fev_buff* evbuff = (fev_buff*)arg;

    do {
        int buf_len = fmbuf_used(evbuff->wbuf);
        if ( buf_len == 0 ) {
            fev_del_event(evbuff->fstate, evbuff->fd, FEV_WRITE);
            break;
        }

        int bytes = fev_write(evbuff->fd, fmbuf_get_head(evbuff->wbuf), buf_len);
        if ( bytes > 0 ) {
            fmbuf_head_seek(evbuff->wbuf, bytes);
        } else if ( bytes == 0 ) { // EAGAIN
            break;
        } else {
            if ( evbuff->error_cb )
                evbuff->error_cb(fev, evbuff, evbuff->arg);
            return;
        }

    } while (1);
}

fev_buff*    fevbuff_new(
                fev_state* fev,
                int fd,
                fev_buff_read read_cb,        // call when the fd can read
                fev_buff_error error_cb,    // call when socket has error
                void* arg)                  // user argument
{
    if ( !fev ) return NULL;

    fev_buff* evbuff = malloc(sizeof(fev_buff));
    if ( !evbuff ) return NULL;

    evbuff->fstate = fev;
    evbuff->fd = fd;
    evbuff->arg = arg;
    evbuff->read_cb = read_cb;
    evbuff->error_cb = error_cb;

    evbuff->rbuf = fmbuf_create(FEV_BUFF_DEFAULT_SIZE);
    if ( !evbuff->rbuf ) {
        free(evbuff);
        return NULL;
    }

    evbuff->wbuf = fmbuf_create(FEV_BUFF_DEFAULT_SIZE);
    if ( !evbuff->wbuf ) {
        fmbuf_delete(evbuff->rbuf);
        free(evbuff);
        return NULL;
    }

    if ( fev_reg_event(fev, fd, FEV_READ, evbuff_read, evbuff_write, evbuff) ) {
        fmbuf_delete(evbuff->rbuf);
        fmbuf_delete(evbuff->wbuf);
        free(evbuff);
        return NULL;
    }

    return evbuff;
}

int     fevbuff_destroy(fev_buff* evbuff)
{
    if ( !evbuff ) return -1;

    int fd = evbuff->fd;
    int mask = FEV_READ | FEV_WRITE;

    fmbuf_delete(evbuff->rbuf);
    fmbuf_delete(evbuff->wbuf);
    fev_del_event(evbuff->fstate, fd, mask);
    free(evbuff);

    return fd;
}

int     fevbuff_get_fd(fev_buff* evbuff)
{
    return evbuff->fd;
}

void*   fevbuff_get_arg(fev_buff* evbuff)
{
    return evbuff->arg;
}

size_t  fevbuff_get_bufflen(fev_buff* evbuff, int type)
{
    if ( type == FEVBUFF_TYPE_READ )
        return fmbuf_size(evbuff->rbuf);
    else
        return fmbuf_size(evbuff->wbuf);
}

size_t  fevbuff_get_usedlen(fev_buff* evbuff, int type)
{
    if ( type == FEVBUFF_TYPE_READ )
        return fmbuf_used(evbuff->rbuf);
    else
        return fmbuf_used(evbuff->wbuf);
}

// if pbuf != NULL, return data_len and copy data to user
// if pbuf == NULL, return data_len without copy data
int     fevbuff_read(fev_buff* evbuff, void* pbuf, size_t len)
{
    size_t used_len = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_READ);
    int need_read_bytes = (int)len - (int)used_len;

    if ( need_read_bytes <= 0 ) {
        if( pbuf ) memcpy(pbuf, fmbuf_get_head(evbuff->rbuf), len);
        return (int)len;
    }

    // rewind or realloc mbuf when tail_free is not enough
    if ( need_read_bytes > (int)fmbuf_tail_free(evbuff->rbuf) ) {
        fmbuf_rewind(evbuff->rbuf);

        if( (int)fmbuf_total_free(evbuff->rbuf) < need_read_bytes ) {
            evbuff->rbuf = fmbuf_realloc(evbuff->rbuf, len);
        }
    }

    int bytes = fev_read(evbuff->fd, fmbuf_get_tail(evbuff->rbuf), need_read_bytes);
    if ( bytes >= 0 ) {
        fmbuf_tail_seek(evbuff->rbuf, bytes);
        size_t used = fmbuf_used(evbuff->rbuf);
        int copy_bytes = used > len ? len : used;
        if( pbuf ) memcpy(pbuf, fmbuf_get_head(evbuff->rbuf), copy_bytes);
        return (int)copy_bytes;
    } else {
        //error
        if( evbuff->error_cb ) 
            evbuff->error_cb(evbuff->fstate, evbuff, evbuff->arg);
        return -1;
    }
}

static
int     fevbuff_cache_data(fev_buff* evbuff, const void* pbuf, size_t len)
{
    if ( len == 0 ) return 0;

    int tail_free = fmbuf_tail_free(evbuff->wbuf);
    if ( tail_free < (int)len ) {
        fmbuf_rewind(evbuff->wbuf);

        tail_free = fmbuf_tail_free(evbuff->wbuf);
        if ( tail_free < (int)len ) {
            evbuff->wbuf = fmbuf_realloc(evbuff->wbuf, len + fmbuf_used(evbuff->wbuf));
        }
    }

    memcpy(fmbuf_get_tail(evbuff->wbuf), pbuf, len);
    // add FEV_WRITE status, because it need to wait for the writing status activitly
    fev_add_event(evbuff->fstate, evbuff->fd, FEV_WRITE);
    return 0;
}

int    fevbuff_write(fev_buff* evbuff, const void* pbuf, size_t len)
{
    if ( !pbuf ) return 0;
    if ( len == 0 ) return 0;

    int bytes = 0;
    fmbuf* wbuf = evbuff->wbuf;
    int write_buf_len = fmbuf_used(wbuf);
    if ( write_buf_len == 0 ) {
        // send immediately
        bytes = fev_write(evbuff->fd, pbuf, len);
        if ( bytes < 0 ) {
            if (evbuff->error_cb) 
                evbuff->error_cb(evbuff->fstate, evbuff, evbuff->arg);
            return -1;
        }
    }

    if ( len - bytes > 0 ) {
        // cache data
        fevbuff_cache_data(evbuff, pbuf+bytes, len-bytes);
    }

    return len;
}

// this interface only operate read buf casue write buf is not visible for user
int     fevbuff_pop(fev_buff* evbuff, size_t len)
{
    if ( len == 0 ) return 0;

    int buf_len = fmbuf_used(evbuff->rbuf);
    int pop_len = buf_len > (int)len ? (int)len : buf_len; 

    fmbuf_head_seek(evbuff->rbuf, pop_len);
    return pop_len;
}

// return readbuff head pointer
void*   fevbuff_rawget(fev_buff* evbuff)
{
    if ( !evbuff ) return NULL;

    return fmbuf_get_head(evbuff->rbuf);
}
