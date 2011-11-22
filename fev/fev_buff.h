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

#define FEVBUFF_TYPE_READ    0
#define FEVBUFF_TYPE_WRITE   1

struct fev_state;
typedef struct fev_buff fev_buff;
typedef void (*fev_buff_read)(fev_buff*, void* arg);
typedef void (*fev_buff_error)(fev_buff*, void* arg);

fev_buff*	fevbuff_new(
                fev_state*,
                int fd,
				fev_buff_read,		// call when the fd can read
				fev_buff_error,	    // call when socket has error
				void* arg);         // user argument

// return fd and unregister from fev_state but not exec close(fd)
int	    fevbuff_destroy(fev_buff* evbuff);

int		fevbuff_get_fd(fev_buff*);
void*   fevbuff_get_arg(fev_buff*);
int		fevbuff_get_bufflen(fev_buff*, int type);
int		fevbuff_get_usedlen(fev_buff*, int type);

int 	fevbuff_read(fev_buff*, void* pbuf, size_t len);        // only read from fd and copy data to local buff, not pop 
int		fevbuff_write(fev_buff*, const char* buff, size_t len);
int     fevbuff_pop(fev_buff*, size_t len); // pop data from local buff
