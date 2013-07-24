/*
 * =====================================================================================
 *
 *       Filename:  fev_timer.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/18/2011 16:59:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  yuzhang hu(finaldie)
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include "ftimer/ftimer.h"
#include "fev_timer.h"

struct fev_timer {
    int fd;
    int once;
    fev_timer_cb callback;
    void* arg;
};

static
void fev_on_timer(fev_state* fev, int fd, int mask, void* arg)
{
    fev_timer* evt = (fev_timer*)arg;

    uint64_t exp;
    while(1) {
        int read_size = read(evt->fd, (char*)&exp, sizeof(uint64_t));
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

// return < 0 : failed
// return > 0(fd) : sucess
fev_timer*  fev_add_timer_event(fev_state* fev, long long nsec, long long alter, fev_timer_cb callback, void* arg)
{
    if ( !fev ) return NULL;

    int fd = ftimerfd_create();
    if ( fd == -1 ) return NULL;

    fev_timer* evt = malloc(sizeof(fev_timer));
    if ( !evt ) return NULL;
    evt->fd = fd;
    evt->once = alter == 0 ? 1 : 0;
    evt->callback = callback;
    evt->arg = arg;

    int mask = FEV_READ;
    int ret = fev_reg_event(fev, fd, mask, fev_on_timer, NULL, evt);
    if ( ret != 0 ){
        printf("fev_timer reg_event failed fd=%d ret=%d mask=%d\n", fd, ret, fev_get_mask(fev, fd));
        close(fd);
        free(evt);
        return NULL;
    } 

    if ( ftimerfd_start(fd, nsec, alter) ) {
        printf("fev_timer start timer failed fd=%d\n", fd);
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
    int mask = FEV_READ | FEV_WRITE;
    if ( !fev ) return -1;

    if ( ftimerfd_stop(evt->fd) )
        return -2;

    int ret = fev_del_event(fev, evt->fd, mask);
    if ( ret != 0 ) return -3;

    close(evt->fd);
    free(evt);
    return 0;
}
