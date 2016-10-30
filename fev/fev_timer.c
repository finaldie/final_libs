#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include "flibs/ftime.h"
#include "flibs/fev_timer.h"

struct fev_timer {
    int fd;
    int once;
    fev_timer_cb callback;
    void* arg;
};

static
void fev_on_timer(fev_state* fev,
                  int fd    __attribute__((unused)),
                  int mask  __attribute__((unused)),
                  void* arg)
{
    fev_timer* evt = (fev_timer*)arg;

    uint64_t exp;
    while(1) {
        ssize_t read_size = read(evt->fd, (char*)&exp, sizeof(uint64_t));
        if ( read_size != sizeof(uint64_t) ) {
            if( errno == EINTR )
                continue;
            return;
        } else {
            if ( evt->callback ) {
                evt->callback(fev, evt->arg);
            }

            if ( evt->once ) {
                fev_del_timer_event(fev, evt);
                return;
            }
        }
    }
}

// return NULL : failed
// return non-NULL : sucess
fev_timer*  fev_add_timer_event(fev_state* fev,
                                long long nsec,  // after how long will start, nano second 
                                long long alter, // interval, nano second
                                fev_timer_cb callback,
                                void* arg)
{
    if (!fev || !callback) return NULL;

    int fd = ftimerfd_create();
    if (fd == -1) return NULL;

    fev_timer* evt = calloc(1, sizeof(fev_timer));
    evt->fd = fd;
    evt->once = alter == 0 ? 1 : 0;
    evt->callback = callback;
    evt->arg = arg;

    int mask = FEV_READ;
    int ret = fev_reg_event(fev, fd, mask, fev_on_timer, NULL, evt);
    if (ret != 0) {
        close(fd);
        free(evt);
        return NULL;
    }

    if (ftimerfd_start(fd, nsec, alter)) {
        fev_del_event(fev, fd, mask);
        close(fd);
        free(evt);
        return NULL;
    }

    return evt;
}

// return 0 : sucess
int     fev_del_timer_event(fev_state* fev, fev_timer* evt)
{
    if ( !fev || !evt ) return -1;
    int mask = FEV_READ | FEV_WRITE;

    if ( ftimerfd_stop(evt->fd) )
        return -2;

    int ret = fev_del_event(fev, evt->fd, mask);
    if ( ret != 0 ) return -3;

    close(evt->fd);
    free(evt);
    return 0;
}
