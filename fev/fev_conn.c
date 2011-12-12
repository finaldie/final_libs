/*
 * =====================================================================================
 *
 *       Filename:  fev_conn.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/23/2011 01:59:17
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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include "fev_conn.h"
#include "net_core.h"
#include "fev_timer.h"

typedef struct fev_conn_info {
    int         fd;
    fev_timer*  timer;
    pfev_conn   conn_cb;
    conn_arg_t  arg;
}fev_conn_info;

static void on_connect(fev_state* fev, int fd, int mask, void* arg)
{
    fev_conn_info* conn_info = (fev_conn_info*)arg;
    fev_del_event(fev, conn_info->fd, FEV_READ | FEV_WRITE);
    assert( fd == conn_info->fd );
    assert( fev_get_mask(fev, fd) == FEV_NIL );
    fev_del_timer_event(fev, conn_info->timer);

    if( mask & FEV_ERROR ) {
        goto CONN_ERROR;
    }
    
    int err = 0;
    socklen_t len = sizeof(int);
    if (( 0 == getsockopt(conn_info->fd, SOL_SOCKET, SO_ERROR, &err, &len) )){
        if( 0 == err ) {
            if( conn_info->conn_cb )
                conn_info->conn_cb(conn_info->fd, conn_info->arg);
        }
        
        goto CONN_END;
    }
    
CONN_ERROR:
    close(conn_info->fd);

    if( conn_info->conn_cb )
        conn_info->conn_cb(-1, conn_info->arg);
CONN_END:
    free(conn_info);
}

static void on_timer(fev_state* fev, void* arg)
{
    fev_conn_info* conn_info = (fev_conn_info*)arg;
    fev_del_event(fev, conn_info->fd, FEV_READ | FEV_WRITE);
    assert( fev_get_mask(fev, fd) == FEV_NIL );
    close(conn_info->fd);

    if( conn_info->conn_cb )
        conn_info->conn_cb(-1, conn_info->arg);

    free(conn_info);
}

void    fev_conn(fev_state* fev, 
            const char* ip, 
            int port, 
            int timeout, 
            pfev_conn pfunc, 
            conn_arg_t arg)
{
    int sockfd = -1;
	int s = net_conn_a(ip, port, &sockfd);
    printf("net_conn_a create fd=%d\\n", sockfd);

	if( s == 0 ){	// connect sucess
        if ( pfunc ) pfunc(sockfd, arg);
	}
	else if( s == -1 ){ // connect error
        if ( pfunc ) pfunc(-1, arg);
	}
	else{
        fev_conn_info* conn_info = (fev_conn_info*)malloc(sizeof(fev_conn_info));
        if( !conn_info ){
            close(sockfd);
            if ( pfunc ) pfunc(-1, arg);
            return;
        }

        conn_info->fd = sockfd;
        conn_info->timer = fev_add_timer_event(fev, (long)timeout * 1000000l, 0, on_timer, conn_info);
        if( !conn_info->timer ) {
            printf("fev_conn init timer failed sockfd=%d\n", sockfd);
            close(sockfd);
            free(conn_info);

            if ( pfunc ) pfunc(-1, arg);
            return;
        }

        conn_info->conn_cb = pfunc;
        conn_info->arg = arg;

        int ret = fev_reg_event(fev, sockfd, FEV_WRITE, NULL, on_connect, conn_info);
        if ( ret != 0 ){
            printf("fev_conn reg_event failed sockfd=%d ret=%d\n", sockfd, ret);
            fev_del_timer_event(fev, conn_info->timer);
            close(sockfd);
            free(conn_info);

            if ( pfunc ) pfunc(-1, arg);
        }
    }
}
