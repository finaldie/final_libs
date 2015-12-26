#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include "flibs/fmbuf.h"
#include "flibs/fev.h"
#include "flibs/fev_buff.h"

#if (EAGAIN != EWOULDBLOCK)
# define NEED_RETRY \
    EAGAIN: \
    case EWOULDBLOCK
#else
# define NEED_RETRY EAGAIN
#endif

#define FEV_BUFF_DEFAULT_SIZE   (1024 * 4)

struct fev_buff {
    int             fd;
#if __WORDSIZE == 64
    int             padding;
#endif

    fev_state*      fstate;
    fmbuf*          rbuf;
    fmbuf*          wbuf;
    fev_buff_read   read_cb;
    fev_buff_error  error_cb;
    void*           arg;
};

static
ssize_t fev_read(int fd, void* pbuf, size_t len)
{
    do {
        ssize_t read_size = read(fd, pbuf, len);

        if (read_size == -1) {
            switch (errno) {
            case EINTR:
                continue;
            case NEED_RETRY:
                return 0;
            default:
                return -1;
            }
        } else if (read_size == 0) {
            return -1;
        } else {
            return read_size;
        }
    } while (1);
}

static
ssize_t fev_write(int fd, const char* pbuf, size_t len)
{
    do {
        ssize_t bytes = write(fd, pbuf, len);

        if (bytes > 0) {
            return bytes;
        } else {
            switch (errno) {
            case EINTR:
                continue;
            case NEED_RETRY:
                return 0;
            default:
                return -1;
            }
        }
    } while (1);
}

static
void evbuff_read(fev_state* fev,
                 int fd     __attribute__((unused)),
                 int mask,
                 void* arg)
{
    fev_buff* evbuff = (fev_buff*)arg;

    if ((mask & FEV_READ) && evbuff->read_cb) {
        evbuff->read_cb(fev, evbuff, evbuff->arg);
        return;
    }

    if ((mask & FEV_ERROR) && evbuff->error_cb) {
        evbuff->error_cb(fev, evbuff, evbuff->arg);
    }
}

static
void evbuff_write(fev_state* fev,
                  int fd    __attribute__((unused)),
                  int mask  __attribute__((unused)),
                  void* arg)
{
    fev_buff* evbuff = (fev_buff*)arg;

    do {
        size_t buf_len = fmbuf_used(evbuff->wbuf);
        if (buf_len == 0) {
            fev_del_event(evbuff->fstate, evbuff->fd, FEV_WRITE);
            break;
        }

        ssize_t bytes = fev_write(evbuff->fd, fmbuf_head(evbuff->wbuf), buf_len);
        if (bytes > 0) {
            fmbuf_head_seek(evbuff->wbuf, (size_t)bytes, FMBUF_SEEK_RIGHT);
        } else if (bytes == 0) { // EAGAIN or EWOULDBLOCK
            break;
        } else {
            if (evbuff->error_cb)
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
    if (!fev) return NULL;

    fev_buff* evbuff = calloc(1, sizeof(fev_buff));

    evbuff->fstate = fev;
    evbuff->fd = fd;
    evbuff->arg = arg;
    evbuff->read_cb = read_cb;
    evbuff->error_cb = error_cb;

    evbuff->rbuf = fmbuf_create(FEV_BUFF_DEFAULT_SIZE);
    if (!evbuff->rbuf) {
        free(evbuff);
        return NULL;
    }

    evbuff->wbuf = fmbuf_create(FEV_BUFF_DEFAULT_SIZE);
    if (!evbuff->wbuf) {
        fmbuf_delete(evbuff->rbuf);
        free(evbuff);
        return NULL;
    }

    if (fev_reg_event(fev, fd, FEV_READ, evbuff_read, evbuff_write, evbuff)) {
        fmbuf_delete(evbuff->rbuf);
        fmbuf_delete(evbuff->wbuf);
        free(evbuff);
        return NULL;
    }

    return evbuff;
}

int     fevbuff_destroy(fev_buff* evbuff)
{
    if (!evbuff) return -1;

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
    if (type == FEVBUFF_TYPE_READ)
        return fmbuf_size(evbuff->rbuf);
    else
        return fmbuf_size(evbuff->wbuf);
}

size_t  fevbuff_get_usedlen(fev_buff* evbuff, int type)
{
    if (type == FEVBUFF_TYPE_READ)
        return fmbuf_used(evbuff->rbuf);
    else
        return fmbuf_used(evbuff->wbuf);
}

// if pbuf != NULL, return data_len and copy data to user
// if pbuf == NULL, return data_len without copy data
ssize_t fevbuff_read(fev_buff* evbuff, void* pbuf, size_t len)
{
    size_t used_len = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_READ);
    used_len = used_len > (size_t)LONG_MAX ? (size_t)LONG_MAX : used_len;

    if (len <= used_len) {
        if (pbuf) {
            memcpy(pbuf, fmbuf_head(evbuff->rbuf), len);
        }
        return (ssize_t)len;
    }

    size_t need_read_bytes = len - used_len;

    // rewind or realloc mbuf when tail_free is not enough
    if (need_read_bytes > fmbuf_tail_free(evbuff->rbuf)) {
        fmbuf_rewind(evbuff->rbuf);

        if (fmbuf_free(evbuff->rbuf) < need_read_bytes) {
            evbuff->rbuf = fmbuf_realloc(evbuff->rbuf, len);
        }
    }

    ssize_t bytes = fev_read(evbuff->fd, fmbuf_tail(evbuff->rbuf), need_read_bytes);
    if (bytes >= 0) {
        fmbuf_tail_seek(evbuff->rbuf, (size_t)bytes, FMBUF_SEEK_RIGHT);
        size_t used = fmbuf_used(evbuff->rbuf);
        size_t copy_bytes = used > len ? len : used;

        if (pbuf) {
            memcpy(pbuf, fmbuf_head(evbuff->rbuf), copy_bytes);
        }

        return (int) copy_bytes;
    } else {
        //error
        if (evbuff->error_cb)
            evbuff->error_cb(evbuff->fstate, evbuff, evbuff->arg);
        return -1;
    }
}

static
int     fevbuff_cache_data(fev_buff* evbuff, const void* pbuf, size_t len)
{
    if (len == 0) return 0;

    size_t tail_free = fmbuf_tail_free(evbuff->wbuf);
    if (tail_free < len) {
        fmbuf_rewind(evbuff->wbuf);

        tail_free = fmbuf_tail_free(evbuff->wbuf);
        if (tail_free < len) {
            evbuff->wbuf = fmbuf_realloc(evbuff->wbuf, len + fmbuf_used(evbuff->wbuf));
        }
    }

    memcpy(fmbuf_tail(evbuff->wbuf), pbuf, len);
    // add FEV_WRITE status, because it need to wait for the writing status activitly
    fev_add_event(evbuff->fstate, evbuff->fd, FEV_WRITE);
    return 0;
}

ssize_t fevbuff_write(fev_buff* evbuff, const void* pbuf, size_t len)
{
    if (!pbuf) return 0;
    if (len == 0) return 0;

    ssize_t bytes = 0;
    fmbuf* wbuf = evbuff->wbuf;
    size_t write_buf_len = fmbuf_used(wbuf);
    if (write_buf_len == 0) {
        // send immediately
        bytes = fev_write(evbuff->fd, pbuf, len);
        if (bytes < 0) {
            if (evbuff->error_cb)
                evbuff->error_cb(evbuff->fstate, evbuff, evbuff->arg);
            return -1;
        }
    }

    size_t unsend_bytes = len - (size_t)bytes;
    if (unsend_bytes > 0) {
        // cache data
        fevbuff_cache_data(evbuff, (const char*)pbuf + bytes, unsend_bytes);
    }

    return bytes;
}

// this interface only operate read buf casue write buf is not visible for user
size_t     fevbuff_pop(fev_buff* evbuff, size_t len)
{
    if (len == 0) return 0;

    size_t buf_len = fmbuf_used(evbuff->rbuf);
    size_t pop_len = buf_len > len ? len : buf_len;

    fmbuf_head_seek(evbuff->rbuf, pop_len, FMBUF_SEEK_RIGHT);
    return pop_len;
}

// return readbuff head pointer
void*   fevbuff_rawget(fev_buff* evbuff)
{
    if (!evbuff) return NULL;

    return fmbuf_head(evbuff->rbuf);
}
