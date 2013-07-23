//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flist.h"
#include "ftu_inc.h"

//TODO...

static pl_mgr plist = NULL;
static int failed_cases = 0;
static int tu_case_num = 0;
int curr_failed_assert = 0;
int curr_total_assert = 0;

typedef struct {
    ftu_init   pfunc;
    char*      case_name;
    char*      describe;
} ftest_case;

void    get_cur_time(my_time* time){
    gettimeofday(&time->tv, NULL);
    //printf("curr_time:%ds %dus\n", (int)time->tv.tv_sec, (int)time->tv.tv_usec);
}

int     get_diff_time(my_time* time1, my_time* time2){
    int diff_sec = time2->tv.tv_sec - time1->tv.tv_sec;
    if ( diff_sec > 0 )
        return diff_sec * 1000000 + time2->tv.tv_usec - time1->tv.tv_usec;
    else
        return time2->tv.tv_usec - time1->tv.tv_usec;
}

void    tu_register_init(){
    if ( plist ) return;
    plist = flist_create();

    tu_case_num = 0;
    failed_cases = 0;
    curr_failed_assert = 0;
    curr_total_assert = 0;
}

void    _tu_register_module(ftu_init pfunc, char* case_name, char* describe){
    tu_case_num++;

    ftest_case* ftc = malloc(sizeof(ftest_case));
    ftc->pfunc = pfunc;
    ftc->case_name = case_name;
    ftc->describe = describe;

    flist_push(plist, ftc);
}

static
int     tu_each_case(ftu_init pfunc)
{
    curr_failed_assert = 0;
    curr_total_assert = 0;

    // run test case 
    pfunc();

    if ( curr_failed_assert ) {
        failed_cases++;
    }

    return 0;
}

void tu_run_cases()
{
    printf("FINAL TEST UNIT START...\n");

    ftest_case* ftc = NULL;
    while ( ( ftc = (ftest_case*)flist_pop(plist) ) ){
        printf("\n <<<<<<< CASE NAME:%s DESCRIBE:%s >>>>>>>\n", 
                ftc->case_name, ftc->describe ? ftc->describe : "");
        tu_each_case(ftc->pfunc);
        free(ftc);

        if ( curr_failed_assert ) {
            printf("[%d ASSERT FAILED -- %d/%d]\n", 
                    curr_failed_assert, 
                    curr_total_assert, 
                    curr_total_assert - curr_failed_assert);
        } else {
            printf("[ALL ASSERT PASSED -- %d/%d]\n",
                    curr_total_assert,
                    curr_total_assert);
        }
    }

    printf("\n--------------------------------------\nTOTAL CASE %d, PASS %d, FAILED %d\n",
            tu_case_num,
            tu_case_num - failed_cases,
            failed_cases);
}
