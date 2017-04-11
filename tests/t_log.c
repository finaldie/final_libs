#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <fcunit.h>
#include "flibs/flog.h"

static flog_file_t* log_handler = NULL;

static
void    _test_log()
{
    FLOG_DEBUG(log_handler, "debug log test");
    FLOG_ERROR(log_handler, "error log test");
    flog_set_level(FLOG_LEVEL_ERROR);
    flog_write(log_handler, "hello world", 11);
    sleep(2);   // wait for log system

    int fd = open("./tests/logs/test_log", O_RDONLY);
    FCUNIT_ASSERT(0 < fd);

    char assert_info[200];
    memset(assert_info, 0, 200);
    ssize_t bytes_read = read(fd, assert_info, 199);
    FCUNIT_ASSERT(0 < bytes_read);

    //printf("read log info:%s\n", assert_info);
    char* ptr = strstr(assert_info, "error log test");
    //printf("find ptr=%p\n", ptr);
    FCUNIT_ASSERT(ptr!=NULL);

    ptr = strstr(assert_info, "hello world");
    FCUNIT_ASSERT(ptr!=NULL);

    close(fd);
}

void    test_sync_log(){
    log_handler = flog_create("./tests/logs/test_log", 0);
    FCUNIT_ASSERT(log_handler);
    _test_log();
    flog_destroy(log_handler);
}

static
void* _test_async_log(void* arg __attribute__((unused)))
{
    FLOG_DEBUG(log_handler, "debug log test");
    FLOG_ERROR(log_handler, "error log test"); // first writen
    flog_set_level(FLOG_LEVEL_DEBUG);
    sleep(2);   // wait for log system
    FLOG_DEBUG(log_handler, "debug log test1"); // second writen
    FLOG_DEBUG(log_handler, "debug log test2"); // will be writen in new file
    flog_write(log_handler, "hello world", 11);
    sleep(2);   // wait for log system

    //printf("try to open file:%s\n", "./tests/logs/test_async_log");
    int fd = open("./tests/logs/test_async_log", O_RDONLY);
    FCUNIT_ASSERT(0 < fd);

    char assert_info[200];
    memset(assert_info, 0, 200);
    ssize_t bytes_read = read(fd, assert_info, 199);
    FCUNIT_ASSERT(0 < bytes_read);

    //printf("read log info:%s\n", assert_info);
    char* ptr = strstr(assert_info, "debug log test");
    //printf("find ptr=%p\n", ptr);
    FCUNIT_ASSERT(ptr!=NULL);

    ptr = strstr(assert_info, "hello world");
    FCUNIT_ASSERT(ptr!=NULL);

    close(fd);

    return NULL;
}

static
void _test_async_event(flog_event_t event)
{
    //printf("receive log event:%u\n", event);

    // we should only receive this event, otherwise there may some errors
    FCUNIT_ASSERT(event == FLOG_EVENT_USER_BUFFER_RELEASED);
}

void test_async_log()
{
    flog_set_roll_size(110);
    flog_set_flush_interval(1);
    flog_set_buffer_size(1024 * 1024);
    size_t buffer_size = flog_get_buffer_size();
    FCUNIT_ASSERT(buffer_size == (1024*1024));
    flog_register_event_callback(_test_async_event);

    log_handler = flog_create("./tests/logs/test_async_log", FLOG_F_ASYNC);
    FCUNIT_ASSERT(log_handler);

    pthread_t tid;
    pthread_create(&tid, NULL, _test_async_log, NULL);
    pthread_join(tid, NULL);
    flog_destroy(log_handler);
}

static
void _test_log_cookie()
{
    flog_set_level(FLOG_LEVEL_INFO);
    flog_set_cookie("this is the log cookie");
    FLOG_INFO(log_handler, "test log cookie");
    sleep(2);

    // open the log file and assert the content
    //printf("try to open file:%s\n", "./tests/logs/test_log_cookie");
    int fd = open("./tests/logs/test_log_cookie", O_RDONLY);
    FCUNIT_ASSERT(fd > 0);

    char assert_info[100];
    memset(assert_info, 0, 100);
    ssize_t bytes_read = read(fd, assert_info, 99);
    FCUNIT_ASSERT(bytes_read > 0);

    //printf("read log info:%s\n", assert_info);
    char* ptr = strstr(assert_info, "[this is the log cookie]");
    //printf("find ptr=%p\n", ptr);
    FCUNIT_ASSERT(ptr != NULL);

    close(fd);
}

void test_log_cookie()
{
    log_handler = flog_create("./tests/logs/test_log_cookie", 0);
    FCUNIT_ASSERT(log_handler);

    _test_log_cookie();
    flog_destroy(log_handler);
}

int main(int argc, char** argv)
{
    FCUNIT_RUN(test_sync_log);
    FCUNIT_RUN(test_async_log);
    FCUNIT_RUN(test_log_cookie);

    return 0;
}
