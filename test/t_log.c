#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tu_inc.h"
#include "log_inc.h"

static flogger* log_handler = NULL;

typedef struct fake_log_file_t {
    FILE*  pf;
    size_t file_size;
    time_t last_flush_time;
    char   pfilename[64];
    char   poutput_filename[128];
    char   filebuf[1024*64];
    size_t ref_count;
}fake_log_file_t;

static
void    _test_log(){
    FLOG_DEBUG(log_handler, "debug log test");
    FLOG_ERROR(log_handler, "error log test");
    flog_set_level(LOG_LEVEL_ERROR);
    sleep(2);   // wait for log system

    fake_log_file_t* ff = (fake_log_file_t*)log_handler;
    int fd = open(ff->poutput_filename, O_RDONLY);
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
    log_handler = flog_create("test_log");
    FTU_ASSERT_EXPRESS(log_handler);
    _test_log();
    flog_destroy(log_handler);

    sleep(2);
}

static
void _test_async_log()
{
    FLOG_DEBUG(log_handler, "debug log test");
    FLOG_ERROR(log_handler, "error log test");
    flog_set_level(LOG_LEVEL_ERROR);
    sleep(2);   // wait for log system

    fake_log_file_t* ff = (fake_log_file_t*)log_handler;
    int fd = open(ff->poutput_filename, O_RDONLY);
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

void test_async_log()
{
    log_handler = flog_create("test_async_log");
    FTU_ASSERT_EXPRESS(log_handler);
    flog_set_mode(FLOG_ASYNC_MODE);
    _test_async_log();
    flog_destroy(log_handler);
    sleep(2);
}
