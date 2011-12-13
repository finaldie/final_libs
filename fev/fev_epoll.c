/*
 * =====================================================================================
 *
 *       Filename:  fev_epoll.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2011/11/13 16/37/50
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <errno.h>
#include <sys/epoll.h>

typedef struct state {
    int epfd;
    struct epoll_event events[FEV_MAX_EVENT_NUM];
}state;

static int fev_state_create(fev_state* fev, int max_ev_size)
{
    state* st = (state*)malloc(sizeof(state));
    if( !st ) return 1;

    st->epfd = epoll_create(max_ev_size);
    if( st->epfd == -1 ) return 2; 

    fev->state = st;
    return 0;
}

static void fev_state_destroy(fev_state* fev)
{
    state* st = fev->state;
    close(st->epfd);
    free(st);
}

// add/mod event
static int fev_state_addevent(fev_state* fev, int fd, int mask)
{
    state* st = fev->state; 
    struct epoll_event ee;
    ee.data.u64 = 0;
    ee.data.fd = fd;
    ee.events = 0;
    
    int op = fev->fevents[fd].mask == FEV_NIL ? 
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    mask |= fev->fevents[fd].mask;   // merge old mask state
    if( mask & FEV_READ ) ee.events |= EPOLLIN;
    if( mask & FEV_WRITE ) ee.events |= EPOLLOUT;

    if ( epoll_ctl(st->epfd, op, fd, &ee) == -1 ) {
        printf("fev_epoll add event fd=%d error:%s\n", fd, strerror(errno));
        return -1;
    }

    fev->fevents[fd].mask = mask;
    return 0;
}   

static int fev_state_delevent(fev_state* fev, int fd, int delmask)
{
    state* st = fev->state; 
    int mask = fev->fevents[fd].mask & (~delmask);   //reserved state except delmask

    struct epoll_event ee;
    ee.data.u64 = 0;
    ee.data.fd = fd;
    ee.events = 0;

    int op = mask == FEV_NIL ? 
            EPOLL_CTL_DEL : EPOLL_CTL_MOD;

    if( mask & FEV_READ ) ee.events |= EPOLLIN;
    if( mask & FEV_WRITE ) ee.events |= EPOLLOUT;

    // we must set it before excute epoll_ctl
    // many library maybe close fd first, So we set our status first also
    fev->fevents[fd].mask = mask;

    if ( epoll_ctl(st->epfd, op, fd, &ee) == -1 ) {
        printf("fev_epoll del event fd=%d error:%s\n", fd, strerror(errno));
        return -1;
    }

    return 0;
}

//return -1 : error
//return >=0 : process event num
static int fev_state_poll(fev_state* fev, int timeout)
{
    state* st = fev->state;
    int nums, i;

    // clear firelist
    memset(fev->firelist, 0, fev->max_ev_size);

    nums = epoll_wait(st->epfd, st->events, FEV_MAX_EVENT_NUM, timeout);
    if( nums < 0 ) {
        if( errno == EINTR )
            return 0;
        return -1;
    }

    int process = 0;
    for(i=0; i< nums; i++){
        struct epoll_event* ee = &(st->events[i]);
		int fd = ee->data.fd;

        // check the fd whether or not in firelist , if in, we ignore it
        // because sometimes we modify another fd state to FEV_NIL, so that we process it unnecessary 
        if( fev->firelist[fd] ) {
            continue;
        }

        int mask = FEV_NIL;
        if( ee->events & EPOLLIN ) mask |= FEV_READ;
        if( ee->events & EPOLLOUT ) mask |= FEV_WRITE;
        if( ee->events & (EPOLLHUP | EPOLLERR) ) mask |= FEV_ERROR;     // FEV_ERROR only used by framework

        if( fev->fevents[fd].pread && (fev->fevents[fd].mask & mask & FEV_READ) ) 
            fev->fevents[fd].pread(fev, fd, mask, fev->fevents[fd].arg);

        if( fev->fevents[fd].pwrite && (fev->fevents[fd].mask & mask & FEV_WRITE) ) 
            fev->fevents[fd].pwrite(fev, fd, mask, fev->fevents[fd].arg);

        process++;
    }

    return process;
}

int fev_state_getfd(fev_state* fev)
{
    state* st = fev->state; 
    return st->epfd;
}
