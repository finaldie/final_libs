/*
 * =====================================================================================
 *
 *       Filename:  fnet_buff.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/18/2011 17:40:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef _FEV_BUFF_H_
#define _FEV_BUFF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "fev.h"

#define FEVBUFF_TYPE_READ    0
#define FEVBUFF_TYPE_WRITE   1

typedef struct fev_buff fev_buff;
typedef void (*fev_buff_read)(fev_state*, fev_buff*, void* arg);
typedef void (*fev_buff_error)(fev_state*, fev_buff*, void* arg);

fev_buff*  fevbuff_new(
                fev_state*,
                int fd,
                fev_buff_read,      // call when the fd can read
                fev_buff_error,     // call when socket has error
                void* arg);         // user argument

// return fd and unregister from fev_state but not exec close(fd)
int        fevbuff_destroy(fev_buff*);
int        fevbuff_get_fd(fev_buff*);
void*      fevbuff_get_arg(fev_buff*);
size_t     fevbuff_get_bufflen(fev_buff*, int type);
size_t     fevbuff_get_usedlen(fev_buff*, int type);

/**
* descriptor:
* only read from fd or copy data to local buff, not pop
*  @ if pbuf != NULL, return data_len and copy data to user
*  @ if pbuf == NULL, return data_len without copy data
* params:
*  @ fev_buff* - the fev_buff
*  @ pbuf - which buff you want to receive data
*  @ len - the length you want to receive
* return:
*  @ the length actually you can fetch in this action
*
* Note: the format of api is similar with the posix read, but to safe use it
*       better to use the len <= max(int), the max buffer size also should limit
*       within max(int)
*/
int     fevbuff_read(fev_buff*, void* pbuf, size_t len);
int     fevbuff_write(fev_buff*, const void* buff, size_t len);
int     fevbuff_pop(fev_buff*, size_t len); // pop data from local buff
void*   fevbuff_rawget(fev_buff*);          // return readbuff head pointer

#ifdef __cplusplus
}
#endif

#endif
