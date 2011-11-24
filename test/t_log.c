#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tu_inc.h"
#include "../base/log_inc.h"

void	_test_log(){
	int i;
	char info[40];
	for( i=0; i<10000; ++i ){
		sprintf(info, "log test%d", i);
		flog(LOG_LEVEL_DEBUG, "test_log.log", "%s\n", info);
	}
}

void	test_log(){
	flog_create();
	_test_log();

	sleep(2);
}
