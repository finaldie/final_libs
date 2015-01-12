#ifndef FEV_CONN_H
#define FEV_CONN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <netinet/in.h>
#include "flibs/fev.h"

typedef union conn_arg_t {
    int      u32;
    uint64_t u64;
    void*    ptr;
}conn_arg_t;

// should call fev_conn() after this
int fev_conn_module_init(fev_state*);

// fd > 0 : sucess
// fd == -1 : error or timeout
typedef void (*fev_conn_cb)(int fd, conn_arg_t arg);

// asynchronous connect method used fev
// unit of timeout : ms
// return 0: connect sucessful or inprocess, you need to wait for callback
// return -1: connect error, there is no need to wait for callback, it won't be called
int    fev_conn(fev_state*,
            const char* ip,
            in_port_t port,
            int timeout,
            fev_conn_cb,
            conn_arg_t);

#ifdef __cplusplus
}
#endif

#endif
