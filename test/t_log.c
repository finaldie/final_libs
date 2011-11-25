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
	sleep(2);   // wait for log system 

    int fd = open("test_log.log", O_RDONLY);
    FTU_ASSERT_GREATER_THAN_INT(0, fd);
    
    char assert_info[40];
    memset(assert_info, 0, 40);
	int bytes_read = read(fd, assert_info, 40);  
    FTU_ASSERT_GREATER_THAN_INT(0, bytes_read);

    printf("read log info:%s\n", assert_info);
    char* ptr = strstr(assert_info, "log test final");
    printf("find ptr=%p\n", ptr);
    FTU_ASSERT_EXPRESS(ptr!=NULL);

    close(fd);
}

void	test_log(){
	flog_create();
	_test_log();

	sleep(2);
}
