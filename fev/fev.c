/*
 * =====================================================================================
 *
 *       Filename:  fev.c
 *
 *    Description:  a light-weight event framework
 *
 *        Version:  1.0
 *        Created:  2011年11月13日 15时59分50秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie 
 *        Company:  
 *
 * =====================================================================================
 */

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

typedef struct fev_fd {
    int fd;
    int mask;   //READ OR WRITE
    void* arg;
}fev_fd;

struct fev{
    fev_fd  fd_events[FEV_MAX_EVENT_NUM];
};

#ifdef __linux__
#include "fev_epoll.c"
#else
#error "only support linux os now!"
#endif

fev*    fev_create()
{
    
}
