#ifndef FEV_LISTEN_H
#define FEV_LISTEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <flibs/fev.h>

typedef struct fev_listen_info fev_listen_info;
typedef void (*fev_accept_cb)(fev_state*, int fd, void* ud);

fev_listen_info* fev_add_listener(fev_state*, in_port_t port, fev_accept_cb, void* ud);
fev_listen_info* fev_add_listener_byfd(fev_state*, int listen_fd, fev_accept_cb,
                                       void* ud);
void fev_del_listener(fev_state*, fev_listen_info*);

#ifdef __cplusplus
}
#endif

#endif

