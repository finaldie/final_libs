/*
 * =============================================================================
 *
 *       Filename:  fev_listen.h
 *
 *    Description:  put listen fd into event framework and wait callback simply 
 *
 *        Version:  1.0
 *        Created:  11/23/2011 16:40:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:
 *
 * =============================================================================
 */

#ifndef _FEV_LISTEN_H_
#define _FEV_LISTEN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "fev.h"

typedef struct fev_listen_info fev_listen_info;
typedef void (*fev_accept_cb)(fev_state*, int fd, void* ud);

fev_listen_info* fev_add_listener(fev_state*, int port, fev_accept_cb, void* ud);
fev_listen_info* fev_add_listener_byfd(fev_state*, int listen_fd, fev_accept_cb,
                                       void* ud);
void fev_del_listener(fev_state*, fev_listen_info*);

#ifdef __cplusplus
}
#endif

#endif
