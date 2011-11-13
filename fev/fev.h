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

typedef struct fev fev;
#define FEV_ADD     0x1
#define FEV_MOD     0x2
#define FEV_DEL     0x4

#define FEV_READ    0x8
#define FEV_WRITE   0x10

typedef (*pfev_process)(fev*, int event, void* arg);

fev* fev_create();
void fev_destroy(fev*);
int  fev_mod(fev*, int fd, int opt, void* arg);
int  fev_poll(fev*, int timeout, pfev_process);

#ifdef __cplusplus
}
#endif

#endif
