#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tu_inc.h"
#include "log_inc.h"

static int log_handler = -1;

void    _test_log(){
    system("rm -f ./test_log.log");

    FLOG_DEBUG(log_handler, "debug log test\n");
    FLOG_ERROR(log_handler, "error log test\n");
    flog_set_level(LOG_LEVEL_ERROR);
    sleep(2);   // wait for log system

    int fd = open("test_log.log", O_RDONLY);
    FTU_ASSERT_GREATER_THAN_INT(0, fd);

    char assert_info[100];
    memset(assert_info, 0, 100);
    int bytes_read = read(fd, assert_info, 100);
    FTU_ASSERT_GREATER_THAN_INT(0, bytes_read);

    printf("read log info:%s\n", assert_info);
    char* ptr = strstr(assert_info, "error log test");
    printf("find ptr=%p\n", ptr);
    FTU_ASSERT_EXPRESS(ptr!=NULL);

    close(fd);
}

void    test_log(){
    log_handler = flog_create("test_log.log");
    FTU_ASSERT_EXPRESS(log_handler >= 0);
    _test_log();

    sleep(2);
}
