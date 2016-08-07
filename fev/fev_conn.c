#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "flibs/fnet.h"
#include "flibs/fev_timer_service.h"
#include "flibs/fev_conn.h"

#define FEV_CONN_MODULE_NAME "__fev_connection_module__"
#define FEV_CONN_TIME_SERVICE_INTERVAL 1

typedef struct fev_conn_info {
    int          fd;
#if __WORDSIZE == 64
    int          padding;
#endif

    ftimer_node* timer;
    fev_conn_cb  conn_cb;
    conn_arg_t   arg;
} fev_conn_info;

static
void    on_connect(fev_state* fev,
                   int fd   __attribute__((unused)),
                   int mask,
                   void* arg)
{
    fev_conn_info* conn_info = (fev_conn_info*)arg;
    fev_del_event(fev, conn_info->fd, FEV_READ | FEV_WRITE);

    if ( mask & FEV_ERROR ) {
        goto CONN_ERROR;
    }

    int err = 0;
    socklen_t len = sizeof(int);
    if (( 0 == getsockopt(conn_info->fd, SOL_SOCKET, SO_ERROR, &err, &len) )) {
        if ( 0 == err ) {
            if ( conn_info->conn_cb ) {
                conn_info->conn_cb(conn_info->fd, conn_info->arg);
            }
            goto CONN_END;
        }
    }

CONN_ERROR:
    close(conn_info->fd);

    if ( conn_info->conn_cb )
        conn_info->conn_cb(-1, conn_info->arg);
CONN_END:
    free(conn_info);
}

static
void    on_timer(fev_state* fev, void* arg)
{
    fev_conn_info* conn_info = (fev_conn_info*)arg;
    fev_del_event(fev, conn_info->fd, FEV_READ | FEV_WRITE);
    close(conn_info->fd);

    if ( conn_info->conn_cb ) {
        conn_info->conn_cb(-1, conn_info->arg);
    }

    free(conn_info);
}

int    fev_conn(fev_state* fev,
            const char* ip,
            in_port_t port,
            int timeout, /* unit ms */
            fev_conn_cb pfunc,
            conn_arg_t arg)
{
    int sockfd = -1;
    int s = fnet_conn_async(ip, port, &sockfd);

    if ( !pfunc ) return -1;

    if ( s == 0 ){    // connect sucess
        if ( pfunc ) pfunc(sockfd, arg);
        return 0;
    } else if ( s == -1 ){ // connect error
        return -1;
    } else {
        fev_conn_info* conn_info = calloc(1, sizeof(fev_conn_info));
        conn_info->fd = sockfd;
        fev_timer_svc* timer_svc = (fev_timer_svc*)fev_get_module_data(fev,
                                        FEV_CONN_MODULE_NAME);
        conn_info->timer = fev_tmsvc_timer_add(timer_svc, (uint32_t)timeout,
                                                on_timer, conn_info);
        if (!conn_info->timer) {
            close(sockfd);
            free(conn_info);
            return -1;
        }

        conn_info->conn_cb = pfunc;
        conn_info->arg = arg;

        int ret = fev_reg_event(fev, sockfd, FEV_WRITE, NULL,
                                on_connect, conn_info);
        if (ret != 0) {
            fev_tmsvc_timer_del(conn_info->timer);
            close(sockfd);
            free(conn_info);
            return -1;
        }

        return 0;
    }
}

void fev_conn_module_unload(fev_state* fev __attribute__((unused)),
                            void* ud)
{
    fev_timer_svc* svc = (fev_timer_svc*)ud;
    fev_tmsvc_destroy(svc);
}

int fev_conn_module_init(fev_state* fev)
{
    if( !fev ) return 1;

    fev_module_t module;
    module.name = FEV_CONN_MODULE_NAME;
    module.fev_module_unload = fev_conn_module_unload;
    module.ud = fev_tmsvc_create(fev, FEV_CONN_TIME_SERVICE_INTERVAL,
                                 FEV_TMSVC_SINGLE_LINKED);
    if( !module.ud ) {
        return 1;
    }

    return fev_register_module(fev, &module);
}
