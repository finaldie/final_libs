#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <fcunit.h>
#include "flibs/fhash.h"
#include "flibs/fev.h"
#include "flibs/fnet.h"
#include "flibs/fev_buff.h"
#include "flibs/fev_listener.h"
#include "flibs/fev_conn.h"
#include "flibs/fev_timer.h"
#include "flibs/fev_timer_service.h"

typedef struct test_arg {
    fev_state* fev;
    int fd;
#if __WORDSIZE == 64
    int padding;
#endif
} test_arg;

typedef struct fake_fev_event {
    int          mask;   //READ OR WRITE
    int          fire_idx;
    fev_read_cb  pread;
    fev_write_cb pwrite;
    void*        arg;
} fake_fev_event;

typedef struct fake_fev_state{
    void*           state;
    fake_fev_event* fevents;
    char*           firelist;
    fhash*          module_tbl;
    int             max_ev_size;
    int             fire_num;
    int             in_processing;
    int             reserved;       // unused
} fake_fev_state;

typedef struct fake_fev_conn_info {
    int         fd;
#if __WORDSIZE == 64
    int         padding;
#endif

    fev_timer*  timer;
    fev_conn_cb conn_cb;
    conn_arg_t  arg;
} fake_fev_conn_info;

void _test_fev_read(fev_state* fev, int fd, int mask, void* arg)
{
    (void)mask;
    test_arg* _arg = (test_arg*)arg;

    FCUNIT_ASSERT(_arg->fev==fev);
    FCUNIT_ASSERT(fd == _arg->fd);
}

void test_fev()
{
    fev_state* fev = fev_create(1024);
    FCUNIT_ASSERT(fev!=NULL);

    fake_fev_state* fake_fev = (fake_fev_state*)fev;
    FCUNIT_ASSERT(0 == fake_fev->fire_num);

    int fd = fnet_listen(NULL, 17758, 100, 0);
    FCUNIT_ASSERT(fd > 0);

    int ret = fev_reg_event(NULL, fd, 0, NULL, NULL, NULL);
    FCUNIT_ASSERT(-1 == ret);

    ret = fev_reg_event(fev, fd, FEV_NIL, NULL, NULL, NULL);
    FCUNIT_ASSERT(-2 == ret);

    ret = fev_reg_event(fev, fd, FEV_NIL | FEV_ERROR, NULL, NULL, NULL);
    FCUNIT_ASSERT(-2 == ret);

    // test for add event before register event
    ret = fev_add_event(fev, fd, FEV_READ);
    FCUNIT_ASSERT(-2 == ret);

    ret = fev_reg_event(fev, fd, FEV_READ, NULL, NULL, NULL);
    FCUNIT_ASSERT(0 == ret);

    // test for duplicate register event
    ret = fev_reg_event(fev, fd, FEV_WRITE, NULL, NULL, NULL);
    FCUNIT_ASSERT(-3 == ret);

    ret = fev_add_event(NULL, fd, 0);
    FCUNIT_ASSERT(-1 == ret);

    ret = fev_add_event(fev, fd, FEV_NIL);
    FCUNIT_ASSERT(0 == ret);

    ret = fev_add_event(fev, fd, FEV_ERROR);
    FCUNIT_ASSERT(0 == ret);

    ret = fev_add_event(fev, fd, FEV_READ);
    FCUNIT_ASSERT(0 == ret);

    ret = fev_add_event(fev, fd, FEV_WRITE);
    FCUNIT_ASSERT(0 == ret);

    ret = fev_del_event(NULL, fd, FEV_WRITE);
    FCUNIT_ASSERT(-1 == ret);

    ret = fev_del_event(fev, fd, FEV_NIL);
    FCUNIT_ASSERT(0 == ret);

    // by now there are two status in fd:FEV_READ & FEV_WRITE
    ret = fev_del_event(fev, fd, FEV_READ);
    FCUNIT_ASSERT(0 == ret);
    FCUNIT_ASSERT(FEV_WRITE == fev_get_mask(fev, fd));

    ret = fev_del_event(fev, fd, FEV_WRITE);
    FCUNIT_ASSERT(0 == ret);
    FCUNIT_ASSERT(FEV_NIL == fev_get_mask(fev, fd));
    FCUNIT_ASSERT(1 == fake_fev->fire_num);
    FCUNIT_ASSERT(0 == fake_fev->fevents[fd].fire_idx);
    FCUNIT_ASSERT(fd == fake_fev->firelist[0]);

    // now the fd has deleted from fev_state
    // so we can retest add event , lookup whether or not sucess
    ret = fev_add_event(fev, fd, FEV_READ);
    FCUNIT_ASSERT(-2 == ret);

    fev_destroy(fev);
    close(fd);
}

static fev_state* g_fev = NULL;
static int start = 0;
static int end = 0;
static fev_listen_info* fli;

static void _test_accept(fev_state* fev, int fd, void* ud)
{
    (void)ud;
    FCUNIT_ASSERT(g_fev==fev);
    FCUNIT_ASSERT(fd > 0);
    close(fd);
}

static void* _test_listener(void* arg)
{
    (void)arg;
    //printf("test listener thread startup\n");
    g_fev = fev_create(1024);
    fli = fev_add_listener(g_fev, 17759, _test_accept, NULL);
    FCUNIT_ASSERT(fli != NULL);

    //printf("wait for poll\n");
    start = 1;
    int process = fev_poll(g_fev, -1);
    FCUNIT_ASSERT(1 == process);

    fev_del_listener(g_fev, fli);
    fev_destroy(g_fev);

    return NULL;
}

void test_fev_listener()
{
    g_fev = NULL;
    fli = NULL;
    start = 0;

    pthread_t tid;
    pthread_create(&tid, 0, _test_listener, NULL);

    while(1) {
        sleep(1);   // wait for fev create completed
        if( start ) break;
    }

    int conn_fd = fnet_conn("127.0.0.1", 17759, 1);
    FCUNIT_ASSERT(conn_fd > 0);

    pthread_join(tid, NULL);

    close(conn_fd);
}

static void buff_read(fev_state* fev, fev_buff* evbuff, void* arg)
{
    (void)arg;
    FCUNIT_ASSERT(fev == g_fev);

    size_t buff_read_len = fevbuff_get_bufflen(evbuff, FEVBUFF_TYPE_READ);
    FCUNIT_ASSERT(0 < buff_read_len);

    size_t buff_read_used = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_READ);
    FCUNIT_ASSERT(0 == buff_read_used);

    char read_buf[20];
    memset(read_buf, 0, 20);
    ssize_t read_size = fevbuff_read(evbuff, read_buf, 20);
    if( read_size > 0 ) {
        buff_read_used = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_READ);
        FCUNIT_ASSERT(buff_read_used == (size_t)read_size);
        //printf("read size=%zu, read_str=%s\n", read_size, read_buf);

        char compare_str[20];
        memset(compare_str, 0, 20);
        snprintf(compare_str, (size_t)read_size, "hello final");
        FCUNIT_ASSERT(0 == strcmp(compare_str, read_buf));

        size_t pop_len = fevbuff_pop(evbuff, (size_t)read_size);
        FCUNIT_ASSERT((size_t)read_size == pop_len);

        char* write_str = "hi final";
        ssize_t write_len = fevbuff_write(evbuff, write_str, 9);
        FCUNIT_ASSERT(9 == write_len);
    } else {
        //error happened
        //printf("error happened haha\n");
    }
}

static void buff_error(fev_state* fev, fev_buff* evbuff, void* arg)
{
    (void)fev;
    (void)arg;
    //printf("evbuff error\n");
    int fd = fevbuff_destroy(evbuff);
    FCUNIT_ASSERT(fd > 0);
    close(fd);

    end = 1;
}

static void fake_accept(fev_state* fev, int fd, void* ud)
{
    (void)ud;
    fev_buff* evbuff = fevbuff_new(fev, fd, buff_read, buff_error, NULL);
    FCUNIT_ASSERT(evbuff != NULL);

    int test_fd = fevbuff_get_fd(evbuff);
    FCUNIT_ASSERT(fd == test_fd);

    void* test_arg = fevbuff_get_arg(evbuff);
    FCUNIT_ASSERT(test_arg == NULL);

    size_t buff_read_len = fevbuff_get_bufflen(evbuff, FEVBUFF_TYPE_READ);
    FCUNIT_ASSERT((1024*4) == buff_read_len);

    size_t buff_write_len = fevbuff_get_bufflen(evbuff, FEVBUFF_TYPE_WRITE);
    FCUNIT_ASSERT((1024*4) == buff_write_len);

    size_t buff_read_used = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_READ);
    FCUNIT_ASSERT(0 == buff_read_used);

    size_t buff_write_used = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_WRITE);
    FCUNIT_ASSERT(0 == buff_write_used);
}

static void* fake_listener(void* arg)
{
    (void)arg;
    g_fev = fev_create(1024);
    fli = fev_add_listener(g_fev, 17759, fake_accept, NULL);
    FCUNIT_ASSERT(fli != NULL);

    //printf("wait for poll\n");
    start = 1;
    while(start){
        fev_poll(g_fev, 500);
    }

    return NULL;
}

void test_fev_buff()
{
    g_fev = NULL;
    fli = NULL;
    start = 0;
    end = 0;

    pthread_t tid;
    pthread_create(&tid, 0, fake_listener, NULL);

    while(1) {
        sleep(1);   // wait for fev create completed
        if( start ) break;
    }

    int conn_fd = fnet_conn("127.0.0.1", 17759, 1);
    FCUNIT_ASSERT(conn_fd > 0);

    char* send_str = "hello final";
    ssize_t send_num = fnet_send_safe(conn_fd, send_str, strlen(send_str)+1);
    FCUNIT_ASSERT(12 == send_num);

    // recv a string
    char recv_buf[20];
    memset(recv_buf, 0, 20);
    ssize_t recv_size = fnet_recv(conn_fd, recv_buf, 20);
    FCUNIT_ASSERT(9 == recv_size);
    //printf("main recv str=%s\n", recv_buf);
    close(conn_fd);

    while(1) {
        if( end ) break;
        sleep(1);
    }

    start = 0;  // let listener thread going down
    pthread_join(tid, NULL);

    fev_del_listener(g_fev, fli);
    fev_destroy(g_fev);
}

static void fake_accept1(fev_state* fev, int fd, void* ud)
{
    (void)fev;
    (void)fd;
    (void)ud;
    //printf("accept sucessful\n");
    //close(fd);
}

static void _test_for_conn(int fd, conn_arg_t arg)
{
    (void)arg;
    //printf("tid=%lu, in async connection callback, time=%ld\n", pthread_self(), time(NULL));
    if( fd <= 0 ) {
        //printf("maybe some error or timer triggered, but the timer shouldn't be triggered!!!, detail: %s\n", strerror(errno));
    }

    FCUNIT_ASSERT(fd > 0);
    FCUNIT_ASSERT(FEV_NIL == fev_get_mask(g_fev, fd));

    close(fd);
    start = 0;
}

static void* fake_listener1(void* arg)
{
    (void)arg;
    g_fev = fev_create(1024);
    FCUNIT_ASSERT( fev_conn_module_init(g_fev) == 0 );
    fli = fev_add_listener(g_fev, 17759, fake_accept1, NULL);
    FCUNIT_ASSERT(fli != NULL);

    //printf("wait for poll\n");
    start = 1;

    conn_arg_t carg;
    //printf("before start async conn, time=%ld\n", time(NULL));
    int ret = fev_conn(g_fev, "127.0.0.1", 17759, 5000, _test_for_conn, carg);
    FCUNIT_ASSERT(0 == ret);

    while(start){
        fev_poll(g_fev, 500);
    }

    fev_del_listener(g_fev, fli);
    fev_destroy(g_fev);
    return NULL;
}

void test_fev_conn()
{
    g_fev = NULL;
    fli = NULL;
    start = 0;
    end = 0;

    //printf("main tid=%lu\n", pthread_self());
    pthread_t tid;
    pthread_create(&tid, 0, fake_listener1, NULL);

    pthread_join(tid, NULL);
}

static void _timeout(fev_state* fev, void* arg)
{
    (void)fev;
    time_t trigger_time = time(NULL);
    time_t start_time = *(time_t*)arg;
    //printf("in timeout, currently start_time = %ld, trigger_time = %ld, diff = %ld\n",
    //       start_time, trigger_time, trigger_time - start_time);

    FCUNIT_ASSERT(trigger_time - start_time >= 2);
    start = 0;
}

void test_timer_service()
{
    // print the resolution of the CLOCK_MONOTONIC_COARSE and CLOCK_MONOTONIC
    struct timespec resolution;
    clock_getres(CLOCK_MONOTONIC_COARSE, &resolution);
    //printf("CLOCK_MONOTONIC_COARSE resolution: %ldns\n", resolution.tv_nsec);

    clock_getres(CLOCK_MONOTONIC, &resolution);
    //printf("CLOCK_MONOTONIC resolution: %ldns\n", resolution.tv_nsec);

    g_fev = NULL;
    g_fev = fev_create(1024);
    FCUNIT_ASSERT(g_fev);

    fev_timer_svc* svc = fev_create_timer_service(g_fev, 1000, /*million second*/
                                                  FEV_TMSVC_SINGLE_LINKED);
    FCUNIT_ASSERT(svc);

    time_t now = time(NULL);
    start = 1;
    ftimer_node* tn = fev_tmsvc_add_timer(svc, 2000, _timeout, &now);
    FCUNIT_ASSERT(tn);

    while (start) {
        fev_poll(g_fev, 500);
    }

    fev_delete_timer_service(svc);
}

int main(int argc, char** argv)
{
    FCUNIT_RUN(test_fev);
    FCUNIT_RUN(test_fev_listener);
    FCUNIT_RUN(test_fev_buff);
    FCUNIT_RUN(test_fev_conn);
    FCUNIT_RUN(test_timer_service);

    return 0;
}
