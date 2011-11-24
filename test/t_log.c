#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tu_inc.h"
#include "log_inc.h"

void	_test_log(){
    system("rm -f ./test_log.log");

	char info[40];
	sprintf(info, "log test final");
	flog(LOG_LEVEL_DEBUG, "test_log.log", "%s\n", info);

    FILE* f = fopen("test_log.log", "r");
    //FTU_ASSERT_GREATER_THAN_INT(0, fd);

    char assert_info[40];
    memset(assert_info, 0, 40);
    fscanf(f, "%s", assert_info);
    FTU_ASSERT_EQUAL_CHAR("log test final", assert_info);
    fclose(f);
}

void	test_log(){
	flog_create();
	_test_log();

	sleep(2);
}