/*
 * =====================================================================================
 *
 *       Filename:  fconn.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/18/2011 17:39:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef _FEV_CONN_H_
#define _FEV_CONN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "fev.h"

typedef union conn_arg_t{
	int		 u32;
	uint64_t u64;
	void*	 ptr;
}conn_arg_t;

// fd > 0 : sucess
// fd == -1 : error or timeout
typedef void (*pfev_conn)(int fd, conn_arg_t arg);

// asynchronous connect method used fev
// unit of timeout : ms
int		fev_conn(fev_state*, 
            const char* ip, 
            int port, 
            int timeout, 
            pfev_conn, 
            conn_arg_t);

#ifdef __cplusplus
}
#endif

#endif
