//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tu_inc.h"
#include "lhash.h"

//TODO...

static f_hash* phash = NULL;
static int failed_cases = 0;
static int tu_case_num = 0;
int curr_failed_assert = 0;

void	get_cur_time(my_time* time){
	gettimeofday(&time->tv, &time->tz);
	//printf("curr_time:%ds %dus\n", (int)time->tv.tv_sec, (int)time->tv.tv_usec);
}

int		get_diff_time(my_time* time1, my_time* time2){
	int diff_sec = time2->tv.tv_sec - time1->tv.tv_sec;
	if ( diff_sec > 0 )
		return diff_sec * 1000000 + time2->tv.tv_usec - time1->tv.tv_usec;
	else
		return time2->tv.tv_usec - time1->tv.tv_usec;
}

void	tu_register_init(){
	if( phash ) return;
	phash = hash_create(0);

    tu_case_num = 0;
    failed_cases = 0;
    curr_failed_assert = 0;
}

void	tu_register_module(char* module_name, pfunc_init pfunc){
    tu_case_num++;
	hash_set_str(phash, module_name, pfunc);
}

pfunc_init	tu_get_module(char* module_name){
	pfunc_init pfunc = hash_get_str(phash, module_name);
	
	return pfunc;
}

static int tu_each_case(void* value)
{
    curr_failed_assert = 0;
    ((pfunc_init)value)();

    if( curr_failed_assert ) {
        failed_cases++;
    }

    return 0;
}

void tu_run_cases()
{
    printf("test unit start...\n");
    hash_foreach(phash, tu_each_case);

    printf("\n--------------------------------------\nTotal case %d, Pass %d, Failed %d\n", 
            tu_case_num,
            tu_case_num - failed_cases,
            failed_cases);
}

/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/
