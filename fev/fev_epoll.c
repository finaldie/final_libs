#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>

typedef struct state {
    int epfd;
    struct epoll_event events[1];
} state;

static
int fev_state_create(fev_state* fev, int max_ev_size)
{
    state* st = calloc(1, sizeof(state) + (size_t)max_ev_size * sizeof(struct epoll_event));

    // the size argument is unused, so 1024 is just a hint for kernel
    st->epfd = epoll_create(1024);
    if (st->epfd == -1) return 1;

    fev->state = st;
    return 0;
}

static
void fev_state_destroy(fev_state* fev)
{
    state* st = fev->state;
    close(st->epfd);
    free(st);
}

// add/mod event
static
int fev_state_addevent(fev_state* fev, int fd, int mask)
{
    state* st = fev->state;
    struct epoll_event ee;
    ee.data.u64 = 0;
    ee.data.fd = fd;
    ee.events = 0;

    int op = fev->fevents[fd].mask == FEV_NIL ?
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    mask |= fev->fevents[fd].mask;   // merge old mask state
    if (mask & FEV_READ) ee.events |= EPOLLIN;
    if (mask & FEV_WRITE) ee.events |= EPOLLOUT;

    if (epoll_ctl(st->epfd, op, fd, &ee) == -1) {
        fprintf(stderr, "fev_epoll add event fd=%d error:%s\n", fd, strerror(errno));
        return -1;
    }

    fev->fevents[fd].mask = mask;
    return 0;
}

static
int fev_state_delevent(fev_state* fev, int fd, int delmask)
{
    state* st = fev->state;
    int mask = fev->fevents[fd].mask & (~delmask);   //reserved state except delmask

    struct epoll_event ee;
    ee.data.u64 = 0;
    ee.data.fd = fd;
    ee.events = 0;

    int op = mask == FEV_NIL ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    if (mask & FEV_READ) ee.events |= EPOLLIN;
    if (mask & FEV_WRITE) ee.events |= EPOLLOUT;

    // we must set it before excute epoll_ctl
    // many library maybe close fd first, So we set our status first also
    fev->fevents[fd].mask = mask;

    if (epoll_ctl(st->epfd, op, fd, &ee) == -1) {
        fprintf(stderr, "fev_epoll del event fd=%d error:%s\n", fd, strerror(errno));
        return -1;
    }

    return 0;
}

//return -1 : error
//return >=0 : process event num
static
int fev_state_poll(fev_state* fev, int event_num, int timeout)
{
    state* st = fev->state;
    int nums, i;

    nums = epoll_wait(st->epfd, st->events, event_num, timeout);
    if (nums < 0) {
        if (errno == EINTR)
            return 0;
        return -1;
    }

    int process = 0;
    for (i = 0; i < nums; i++) {
        struct epoll_event* ee = &(st->events[i]);
        int fd = ee->data.fd;

        // check the fd whether or not in firelist , if in, we ignore it
        // because sometimes we modify another fd state to FEV_NIL, so that
        // we process it unnecessary
        if (fev_is_fired(fev, fd)) {
            continue;
        }

        int mask = FEV_NIL;
        if (ee->events & EPOLLIN) mask |= FEV_READ;
        if (ee->events & EPOLLOUT) mask |= FEV_WRITE;
        if (ee->events & (EPOLLHUP | EPOLLERR)) mask |= FEV_ERROR;

        // Some OS releases emit the error with the EPOLLIN or EPOLLOUT together,
        //  but some of them not, which only emit the EPOLLHUP or EPOLLERR.
        // So to trigger the read/write callback correct if there is error occurred,
        //  here fev will try to trigger read callback if possible, if it's done,
        //  then no need to trigger write callback; If no read callback can be
        //  triggered, then fev will trigger write callback directly. In any of
        //  cases, user can handle the error either in read or write callback
        //  only once.
        if (fev->fevents[fd].pread &&
            ((fev->fevents[fd].mask & mask & FEV_READ) || (mask & FEV_ERROR))) {
            fev->fevents[fd].pread(fev, fd, mask, fev->fevents[fd].arg);

            if (mask & FEV_ERROR) continue;
        }

        if (fev->fevents[fd].pwrite &&
            ((fev->fevents[fd].mask & mask & FEV_WRITE) || (mask & FEV_ERROR))) {
            fev->fevents[fd].pwrite(fev, fd, mask, fev->fevents[fd].arg);
        }

        process++;
    }

    return process;
}

int fev_state_getfd(fev_state* fev)
{
    state* st = fev->state;
    return st->epfd;
}
