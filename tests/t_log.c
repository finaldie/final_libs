#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "ftu_inc.h"
#include "flog_inc.h"
#include "inc.h"

static log_file_t* log_handler = NULL;

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
    log_file_write(log_handler, NULL, 0, "hello world", 11);
    sleep(2);   // wait for log system

    fake_log_file_t* ff = (fake_log_file_t*)log_handler;
    int fd = open(ff->poutput_filename, O_RDONLY);
    FTU_ASSERT_GT_INT(0, fd);

    char assert_info[100];
    memset(assert_info, 0, 100);
    int bytes_read = read(fd, assert_info, 100);
    FTU_ASSERT_GT_INT(0, bytes_read);

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
void* _test_async_log(void* arg __attribute__((unused)))
{
    FLOG_DEBUG(log_handler, "debug log test");
    FLOG_ERROR(log_handler, "error log test"); // first writen
    flog_set_level(LOG_LEVEL_DEBUG);
    sleep(2);   // wait for log system
    FLOG_DEBUG(log_handler, "debug log test1"); // second writen
    FLOG_DEBUG(log_handler, "debug log test2"); // will be writen in new file
    log_file_write(log_handler, NULL, 0, "hello world", 11);
    sleep(2);   // wait for log system

    fake_log_file_t* ff = (fake_log_file_t*)log_handler;
    printf("try to open file:%s\n", ff->poutput_filename);
    int fd = open(ff->poutput_filename, O_RDONLY);
    FTU_ASSERT_GT_INT(0, fd);

    char assert_info[100];
    memset(assert_info, 0, 100);
    int bytes_read = read(fd, assert_info, 100);
    FTU_ASSERT_GT_INT(0, bytes_read);

    printf("read log info:%s\n", assert_info);
    char* ptr = strstr(assert_info, "debug log test");
    printf("find ptr=%p\n", ptr);
    FTU_ASSERT_EXPRESS(ptr!=NULL);

    close(fd);

    return NULL;
}

static
void _test_async_event(LOG_EVENT event)
{
    printf("receive log event:%u\n", event);
}

void test_async_log()
{
    flog_set_mode(LOG_ASYNC_MODE);
    flog_set_roll_size(100);
    flog_set_flush_interval(1);
    flog_set_buffer_size(1024 * 1024);
    size_t buffer_size = flog_get_buffer_size();
    FTU_ASSERT_EXPRESS(buffer_size == (1024*1024));
    flog_register_event_callback(_test_async_event);

    log_handler = flog_create("test_async_log");
    FTU_ASSERT_EXPRESS(log_handler);

    pthread_t tid;
    pthread_create(&tid, NULL, _test_async_log, NULL);
    pthread_join(tid, NULL);
    flog_destroy(log_handler);
}
