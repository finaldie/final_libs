/*
 * =====================================================================================
 *
 *       Filename:  fev.h
 *
 *    Description:  a light-weight event framework
 *
 *        Version:  1.0
 *        Created:  11/13/2011 15:15:12
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef _FEV_H_
#define _FEV_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fev_state fev_state;
#define FEV_ADD     0x1
#define FEV_MOD     0x2
#define FEV_DEL     0x4

// fev fd state
#define FEV_NIL     0x0
#define FEV_READ    0x1
#define FEV_WRITE   0x2

// fev event type
#define FEV_IO      0x1
#define FEV_TIMER   0x2

typedef void (*pfev_process)(fev_state*, int fd, void* arg, int mask);

fev_state* fev_create();
void fev_destroy(fev_state*);
int  fev_poll(fev_state*, int timeout);

// the two category interfaces as follow return fd
int  fev_add_io_event(fev_state*, int fd, int mask, pfev_process, void* arg);
int  fev_del_io_event(fev_state*, int fd, int mask);
int  fev_add_timer_event(fev_state*, long long nsec, long long alter, pfev_process, void* arg);
int  fev_del_timer_event(fev_state*, int fd);

#ifdef __cplusplus
}
#endif

#endif
