//base info: create by hyz
/*effect: tu inc
*
*
*/


#ifndef _TEST_UNIT_INC_H_
#define _TEST_UNIT_INC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>

/*
#define fimport(module_name)	\
	##include "module_name.h"	\
	register_module(#module_name, module_name_init);
	*/

// test use this define
#define TRUN(func) \
	do{\
		pid_t f = fork();\
		if( f == 0 ){	\
			printf("test start-module:%s\n", #func);	\
			func();\
			printf("test end-module:%s\n", #func);	\
			exit(1);	\
		}	\
		else{\
			int status;	\
			wait(&status);	\
		}\
	}while(0);


typedef void (*pfunc_init)();

void	tu_register_init();
void	_tu_register_module(pfunc_init pfunc, char* case_name, char* describe);
#define tu_register_module(pfunc, describe) \
    _tu_register_module(pfunc, #pfunc, describe);
    
void    tu_run_cases();

typedef struct{
	struct timeval tv;
	struct timezone tz;
}my_time;

void get_cur_time(my_time*);
int  get_diff_time(my_time* time1, my_time* time2);

extern int curr_failed_assert;
extern int curr_total_assert;

// ASSERT MACROS
#define FTU_ASSERT_EQUAL_CHAR(expect, real) \
    do{ curr_total_assert++; if( strcmp(expect, real) ) { printf("(%s %s) %d: ASSERT FAILED, expect=%s but real=%s \n", __FILE__, __func__, __LINE__, expect, real); curr_failed_assert++; } }while(0)

#define FTU_ASSERT_EQUAL_INT(expect, real) \
    do{ curr_total_assert++; if( expect != real ) { printf("(%s %s) %d: ASSERT FAILED, expect=%d but real=%d \n", __FILE__, __func__, __LINE__, expect, real); curr_failed_assert++; } }while(0)

#define FTU_ASSERT_EQUAL_DOUBLE(expect, real) \
    do{ curr_total_assert++; if( fabs(expect - real) < 0.0000001 ) { printf("(%s %s) %d: ASSERT FAILED, expect=%f but real=%f \n", __FILE__, __func__, __LINE__, expect, real); curr_failed_assert++; } }while(0)

#define FTU_ASSERT_GREATER_THAN_INT(expect, real) \
    do{ curr_total_assert++; if( real < expect ) { printf("(%s %s) %d: ASSERT FAILED, expect > %d but real=%d \n", __FILE__, __func__, __LINE__, expect, real); curr_failed_assert++; } }while(0)

#define FTU_ASSERT_LESS_THAN_INT(expect, real) \
    do{ curr_total_assert++; if( real > expect ) { printf("(%s %s) %d: ASSERT FAILED, expect < %d but real=%d \n", __FILE__, __func__, __LINE__, expect, real); curr_failed_assert++; } }while(0)

#define FTU_ASSERT_EXPRESS(express) \
    do{ curr_total_assert++; if( !(express) ) { printf("(%s %s) %d: ASSERT FAILED, expect=%s but failed \n", __FILE__, __func__, __LINE__, #express); curr_failed_assert++; } }while(0)

#ifdef __cplusplus
}
#endif

#endif

