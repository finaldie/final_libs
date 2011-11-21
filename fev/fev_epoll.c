/*
 * =====================================================================================
 *
 *       Filename:  fev_epoll.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2011年11月13日 16时37分50秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#include <sys/epoll.h>

typedef struct state {
    int epfd;
    struct epoll_event events[FEV_MAX_EVENT_NUM];
}state;

static int fev_state_create(fev_state* fev)
{
    state* st = (state*)malloc(sizeof(state));
    if( !st ) return 1;

    st->epfd = epoll_create(1024);
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
    
    int op = fevents[fd].mask == FEV_NIL ? 
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    mask |= fevents[fd].mask;   // merge old mask state
    if( mask & FEV_READ ) ee.events |= EPOLLIN;
    if( mask & FEV_WRITE ) ee.events |= EPOLLOUT;

    if ( epoll_ctl(st->epfd, op, fd, &ee) == -1 ) return -1;
    return 0;
}   

static int fev_state_delevent(fev_state* fev, int fd, int delmask)
{
    state* st = fev->state; 
    int mask = fevents[fd].mask & (~delmask);   //reserved state except delmask
    struct epoll_event ee;
    ee.data.u64 = 0;
    ee.data.fd = fd;
    ee.events = 0;

    int op = mask == FEV_NIL ? 
            EPOLL_CTL_DEL : EPOLL_CTL_MOD;

    if( mask & FEV_READ ) ee.events |= EPOLLIN;
    if( mask & FEV_WRITE ) ee.events |= EPOLLOUT;

    if ( epoll_ctl(st->epfd, op, fd, &ee) == -1 ) return -1;
    return 0;
}

//return -1 : error
//return >=0 : process event num
static int fev_state_poll(fev_state* fev, int timeout)
{
    state* st = fev->state;
    int nums, i;

    nums = epoll_wait(st->epfd, st->events, FEV_MAX_EVENT_NUM, timeout);
    if( nums < 0 ) {
        if( errno == EINTR )
            return 0;
        return -1;
    }

    for(i=0; i< nums; i++){
        struct epoll_event* ee = &(st->events[i]);

        int mask = FEV_NIL;
        if( ee->events & EPOLLIN ) mask |= FEV_READ;
        if( ee->events & EPOLLOUT ) mask |= FEV_WRITE;

		int fd = ee->data.fd;
        if( fevents[fd].pread && (fevents[fd].mask & mask & FEV_READ) ) 
            fevents[fd].pread(fev, fd, mask, fevents[fd].arg);

        if( fevents[fd].pwrite && (fevents[fd].mask & mask & FEV_WRITE) ) 
            fevents[fd].pwrite(fev, fd, mask, fevents[fd].arg);
    }

    return nums;
}
