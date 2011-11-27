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
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "fev_buff.h"
#include "fev.h"
#include "mbuf.h"

#define FEV_BUFF_DEFAULT_SIZE   (1024 * 4)

struct fev_buff {
    int             fd;
    fev_state*      fstate;
    mbuf*           rbuf;
    mbuf*           wbuf;
    fev_buff_read   read_cb;
    fev_buff_error  error_cb;
    void*           arg;
};

static int fev_read(int fd, void* pbuf, size_t len)
{
    do{
		int read_size = read(fd, pbuf, len);

		if ( read_size == -1 )
		{
			if( errno == EINTR )
				continue;
			else if( errno == EAGAIN )
				return 0;   // we can ignore this and simply to return 0
			return -1;
		}
		else if( read_size == 0 )
			return -1;
		else
			return read_size;
	}while(1);
}

static int fev_write(int fd, const char* pbuf, size_t len)
{
    do{
		int bytes = write(fd, pbuf, len);

		if( bytes > 0 )
			return bytes;
		else
		{
			if( errno == EINTR )
				continue;
			else if( errno == EAGAIN )
				return 0;
			else
				return -1;
		}
	}while(1);
}

static void evbuff_read(fev_state* fev, int fd, int mask, void* arg)
{
    fev_buff* evbuff = (fev_buff*)arg;
    if( evbuff->read_cb ) 
        evbuff->read_cb(fev, evbuff, evbuff->arg);
}

static void evbuff_write(fev_state* fev, int fd, int mask, void* arg)
{
    fev_buff* evbuff = (fev_buff*)arg;

    do{
        int buf_len = mbuf_used(evbuff->wbuf);
        if ( buf_len == 0 ) {
            fev_del_event(evbuff->fstate, evbuff->fd, FEV_WRITE);
            break;
        }

        int bytes = fev_write(evbuff->fd, mbuf_get_head(evbuff->wbuf), buf_len);
        if( bytes > 0 ) {
            mbuf_head_seek(evbuff->wbuf, bytes);
        }
        else if( bytes == 0 ) { // EAGAIN
            break;
        }  
        else {
            if ( evbuff->error_cb ) 
                evbuff->error_cb(fev, evbuff, evbuff->arg);
        }

    }while(1);
}

fev_buff*	fevbuff_new(
                fev_state* fev,
                int fd,
				fev_buff_read read_cb,		// call when the fd can read
				fev_buff_error error_cb,	// call when socket has error
				void* arg)                  // user argument
{
    if( !fev ) return NULL;

    fev_buff* evbuff = (fev_buff*)malloc(sizeof(fev_buff));
    if( !evbuff ) return NULL;

    evbuff->fstate = fev;
    evbuff->fd = fd;
    evbuff->arg = arg;
    evbuff->read_cb = read_cb;
    evbuff->error_cb = error_cb;

    evbuff->rbuf = mbuf_create(FEV_BUFF_DEFAULT_SIZE);
    if( !evbuff->rbuf ) {
        free(evbuff);
        return NULL;
    }

    evbuff->rbuf = mbuf_create(FEV_BUFF_DEFAULT_SIZE);
    if( !evbuff->wbuf ) {
        mbuf_delete(evbuff->rbuf);
        free(evbuff);
        return NULL;
    }

    if ( fev_reg_event(fev, fd, FEV_READ, evbuff_read, evbuff_write, evbuff) ) {
        mbuf_delete(evbuff->rbuf);
        mbuf_delete(evbuff->wbuf);
        free(evbuff);
        return NULL;
    }

    return evbuff;
}

int  fevbuff_destroy(fev_buff* evbuff)
{
    if( !evbuff ) return -1;

    int fd = evbuff->fd;
    int mask = FEV_READ | FEV_WRITE;

    mbuf_delete(evbuff->rbuf);
    mbuf_delete(evbuff->wbuf);
    fev_del_event(evbuff->fstate, fd, mask);
    free(evbuff);

    return fd;
}

int fevbuff_get_fd(fev_buff* evbuff)
{
    return evbuff->fd;
}

void*   fevbuff_get_arg(fev_buff* evbuff)
{
    return evbuff->arg;
}

int fevbuff_get_bufflen(fev_buff* evbuff, int type)
{
    if( type == FEVBUFF_TYPE_READ )
        return mbuf_size(evbuff->rbuf);
    else
        return mbuf_size(evbuff->wbuf);
}

int fevbuff_get_usedlen(fev_buff* evbuff, int type)
{
    if( type == FEVBUFF_TYPE_READ )
        return mbuf_used(evbuff->rbuf);
    else
        return mbuf_used(evbuff->wbuf);
}

int fevbuff_read(fev_buff* evbuff, void* pbuf, size_t len)
{
    int used_len = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_READ);
    int need_read_bytes = len - used_len;

    if( need_read_bytes <= 0 ) {
        memcpy(mbuf_get_head(evbuff->rbuf), pbuf, len);
        return (int)len;
    }
    
    // rewind or realloc mbuf when tail_free is not enough
    if ( need_read_bytes > mbuf_tail_free(evbuff->rbuf) ) {
        mbuf_rewind(evbuff->rbuf);

        if( mbuf_total_free(evbuff->rbuf) < need_read_bytes ) {
            evbuff->rbuf = mbuf_realloc(evbuff->rbuf, len);
        }
    }

    int bytes = fev_read(evbuff->fd, mbuf_get_tail(evbuff->rbuf), need_read_bytes);
    if ( bytes >= 0 ) {
        mbuf_tail_seek(evbuff->rbuf, bytes);
        int used = mbuf_used(evbuff->rbuf);
        int copy_bytes = used > len ? len : used;
        memcpy(mbuf_get_head(evbuff->rbuf), pbuf, copy_bytes);
        return (int)copy_bytes;
    }
    else {
        //error
        if( evbuff->error_cb ) 
            evbuff->error_cb(evbuff->fstate, evbuff, evbuff->arg);
        return -1;
    }
}

static int fevbuff_cache_data(fev_buff* evbuff, const void* pbuf, size_t len)
{
    if( len == 0 ) return 0;

    int tail_free = mbuf_tail_free(evbuff->wbuf);
    if( tail_free < (int)len ){
        mbuf_rewind(evbuff->wbuf);

        tail_free = mbuf_tail_free(evbuff->wbuf);
        if( tail_free < (int)len ){
            evbuff->wbuf = mbuf_realloc(evbuff->wbuf, len + mbuf_used(evbuff->wbuf));
        }
    }

    memcpy(mbuf_get_tail(evbuff->wbuf), pbuf, len);
    return 0;
}

int	fevbuff_write(fev_buff* evbuff, const void* pbuf, size_t len)
{
    if( !pbuf ) return 0;
    if( len == 0 ) return 0;

    int bytes = 0;
    mbuf* wbuf = evbuff->wbuf;
	int write_buf_len = mbuf_used(wbuf);
    if( write_buf_len == 0 ){
        // send immediately
        bytes = fev_write(evbuff->fd, pbuf, len);
        if( bytes < 0 ){
            if(evbuff->error_cb) 
                evbuff->error_cb(evbuff->fstate, evbuff, evbuff->arg);
            return -1;
        }
    }
    
    // 1.cache data
    // 2.modify fd state
    fevbuff_cache_data(evbuff, pbuf+bytes, len-bytes);
    fev_add_event(evbuff->fstate, evbuff->fd, FEV_WRITE);
    return len;
}

// this interface only operate read buf casue write buf is not visible for user
int fevbuff_pop(fev_buff* evbuff, size_t len)
{
    if( len == 0 ) return 0;

    int buf_len = mbuf_used(evbuff->rbuf);
    int pop_len = buf_len > (int)len ? (int)len : buf_len; 

    mbuf_head_seek(evbuff->rbuf, pop_len);
    return pop_len;
}
