#include <stdlib.h>
#include <unistd.h>

#include "flibs/fnet.h"
#include "flibs/fev_listener.h"

#define FEV_LISTEN_QUEUE_NUM 1024

struct fev_listen_info {
    int           fd;
#if __WORDSIZE == 64
    int           padding;
#endif

    fev_accept_cb accept_cb;
    void*         ud;
};

static void on_listen_port(fev_state* fev,
                            int fd      __attribute__((unused)),
                            int mask    __attribute__((unused)),
                            void* arg)
{
    fev_listen_info* listen_info = (fev_listen_info*)arg;

    while(1) {
        int new_fd = fnet_accept(listen_info->fd);
        if( new_fd < 0 ) return;

        if ( listen_info->accept_cb )
            listen_info->accept_cb(fev, new_fd, listen_info->ud);
    }
}

fev_listen_info* fev_add_listener(fev_state* fev,
        in_port_t port, fev_accept_cb accept_cb, void* ud)
{
    if( !fev ) return NULL;

    fev_listen_info* listen_info = calloc(1, sizeof(fev_listen_info));

    int listen_fd = fnet_listen(NULL, port, FEV_LISTEN_QUEUE_NUM, 0);
    if( listen_fd < 0 ) {
        free(listen_info);
        return NULL;
    }

    listen_info->fd = listen_fd;
    listen_info->accept_cb = accept_cb;
    listen_info->ud = ud;

    int ret = fev_reg_event(fev, listen_info->fd, FEV_READ, on_listen_port, NULL, listen_info);
    if( ret < 0 ) {
        free(listen_info);
        return NULL;
    }

    return listen_info;
}

fev_listen_info* fev_add_listener_byfd(fev_state* fev, int listen_fd,
                                  fev_accept_cb accept_cb, void* ud)
{
    if( !fev ) return NULL;
    if( listen_fd < 0 || !accept_cb) return NULL;

    fev_listen_info* listen_info = calloc(1, sizeof(fev_listen_info));

    listen_info->fd = listen_fd;
    listen_info->accept_cb = accept_cb;
    listen_info->ud = ud;

    int ret = fev_reg_event(fev, listen_info->fd, FEV_READ, on_listen_port, NULL, listen_info);
    if( ret < 0 ) {
        free(listen_info);
        return NULL;
    }

    return listen_info;
}

void fev_del_listener(fev_state* fev, fev_listen_info* listen_info)
{
    if( !fev || !listen_info ) return;

    fev_del_event(fev, listen_info->fd, FEV_READ | FEV_WRITE );
    close(listen_info->fd);
    free(listen_info);
}
