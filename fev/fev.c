/*
 * =====================================================================================
 *
 *       Filename:  fev.c
 *
 *    Description:  a light-weight event framework
 *
 *        Version:  1.0
 *        Created:  2011/11/13 15/59/50
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie 
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include "fev.h"

#define FEV_MAX_EVENT_NUM   (1024 * 10)

typedef struct fev_event {
    int         mask;   //READ OR WRITE
    pfev_read   pread;
    pfev_write  pwrite;
    void*       arg;
}fev_event;

struct fev_state{
    void*       state;
};

static fev_event fevents[FEV_MAX_EVENT_NUM];

#ifdef __linux__
#include "fev_epoll.c"
#else
#error "only support linux os now!"
#endif

fev_state*    fev_create()
{
    fev_state* fev = (fev_state*)malloc(sizeof(fev_state));   
    if( !fev ){
        perror("fev create malloc");
        return NULL;
    }

    if( fev_state_create(fev) ) {
        perror("fev create state");
        free(fev);
        return NULL;
    }

    return fev;
}

void    fev_destroy(fev_state* fev)
{
    if( !fev ) return;

    fev_state_destroy(fev->state);
    free(fev);
}

// return -1 : fev is null
// return -2 : reg event failed
// return > 0 : sucess
int     fev_reg_event(fev_state* fev, int fd, int mask, pfev_read pread, pfev_write pwrite, void* arg)
{
    if( !fev ) return -1; 

    if( fev_state_addevent(fev, fd, mask) == -1 ) 
        return -2;

    fevents[fd].mask = mask;
    fevents[fd].pread = pread;
    fevents[fd].pwrite = pwrite;
    fevents[fd].arg = arg;

    return 0;
}

int fev_add_event(fev_state* fev, int fd, int mask)
{
    if( !fev ) return -1; 

    if( fev_state_addevent(fev, fd, mask) == -1 ) 
        return -2;

    fevents[fd].mask = mask;
    return 0;
}

// return -1 : fev is null
// return -2 : del event failed
// return > 0 : sucess
int     fev_del_event(fev_state* fev, int fd, int mask)
{
    if( !fev ) return -1;

    if( fev_state_delevent(fev, fd, mask) == -1 )
        return -2;

    fevents[fd].mask = mask;
    return 0;
}

int     fev_poll(fev_state* fev, int timeout)
{
    if( !fev ) return 0;

    int num = fev_state_poll(fev, timeout);

    return num;
}
