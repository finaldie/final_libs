/*
 * =====================================================================================
 *
 *       Filename:  t_fev.c
 *
 *    Description:  test fev
 *
 *        Version:  1.0
 *        Created:  11/26/2011 12:29:56
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:  
 *
 * =====================================================================================
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "ftu_inc.h"
#include "fhash.h"
#include "fev.h"
#include "fnet_core.h"
#include "fev_buff.h"
#include "fev_listener.h"
#include "fev_conn.h"
#include "fev_timer.h"
#include "inc.h"

typedef struct {
    fev_state* fev;
    int fd;
}test_arg;

typedef struct fake_fev_event {
    int         mask;   //READ OR WRITE
    int         fire_idx;
    pfev_read   pread;
    pfev_write  pwrite;
    void*       arg;
}fake_fev_event;

typedef struct fake_fev_state{
    void*           state;
    fake_fev_event* fevents;
    char*           firelist;
    fhash*          module_tbl;
    int             max_ev_size;
    int             fire_num;
    int             in_processing;
}fake_fev_state;

typedef struct fake_fev_conn_info {
    int         fd;
    fev_timer*  timer;
    pfev_conn   conn_cb;
    conn_arg_t  arg;
}fake_fev_conn_info;

void test_fev_read(fev_state* fev, int fd, int mask, void* arg)
{
    test_arg* _arg = (test_arg*)arg;

    FTU_ASSERT_EXPRESS(_arg->fev==fev);
    FTU_ASSERT_EQUAL_INT(fd, _arg->fd);
}

void test_fev()
{
    fev_state* fev = fev_create(1024);
    FTU_ASSERT_EXPRESS(fev!=NULL);

    fake_fev_state* fake_fev = (fake_fev_state*)fev;
    FTU_ASSERT_EQUAL_INT(0, fake_fev->fire_num);

    int fd = fnet_create_listen(NULL, 17758, 100, 0);
    FTU_ASSERT_GREATER_THAN_INT(0, fd);

    int ret = fev_reg_event(NULL, fd, 0, NULL, NULL, NULL);
    FTU_ASSERT_EQUAL_INT(-1, ret);

    ret = fev_reg_event(fev, fd, FEV_NIL, NULL, NULL, NULL);
    FTU_ASSERT_EQUAL_INT(-2, ret);

    ret = fev_reg_event(fev, fd, FEV_NIL | FEV_ERROR, NULL, NULL, NULL);
    FTU_ASSERT_EQUAL_INT(-2, ret);

    // test for add event before register event
    ret = fev_add_event(fev, fd, FEV_READ);
    FTU_ASSERT_EQUAL_INT(-2, ret);

    ret = fev_reg_event(fev, fd, FEV_READ, NULL, NULL, NULL);
    FTU_ASSERT_EQUAL_INT(0, ret);

    // test for duplicate register event
    ret = fev_reg_event(fev, fd, FEV_WRITE, NULL, NULL, NULL);
    FTU_ASSERT_EQUAL_INT(-3, ret);

    ret = fev_add_event(NULL, fd, 0);
    FTU_ASSERT_EQUAL_INT(-1, ret);

    ret = fev_add_event(fev, fd, FEV_NIL);
    FTU_ASSERT_EQUAL_INT(0, ret);

    ret = fev_add_event(fev, fd, FEV_ERROR);
    FTU_ASSERT_EQUAL_INT(0, ret);

    ret = fev_add_event(fev, fd, FEV_READ);
    FTU_ASSERT_EQUAL_INT(0, ret);

    ret = fev_add_event(fev, fd, FEV_WRITE);
    FTU_ASSERT_EQUAL_INT(0, ret);

    ret = fev_del_event(NULL, fd, FEV_WRITE);
    FTU_ASSERT_EQUAL_INT(-1, ret);

    ret = fev_del_event(fev, fd, FEV_NIL);
    FTU_ASSERT_EQUAL_INT(0, ret);

    // by now there are two status in fd:FEV_READ & FEV_WRITE
    ret = fev_del_event(fev, fd, FEV_READ);
    FTU_ASSERT_EQUAL_INT(0, ret);
    FTU_ASSERT_EQUAL_INT(FEV_WRITE, fev_get_mask(fev, fd));

    ret = fev_del_event(fev, fd, FEV_WRITE);
    FTU_ASSERT_EQUAL_INT(0, ret);
    FTU_ASSERT_EQUAL_INT(FEV_NIL, fev_get_mask(fev, fd));
    FTU_ASSERT_EQUAL_INT(1, fake_fev->fire_num);
    FTU_ASSERT_EQUAL_INT(0, fake_fev->fevents[fd].fire_idx);
    FTU_ASSERT_EQUAL_INT(fd, fake_fev->firelist[0]);

    // now the fd has deleted from fev_state
    // so we can retest add event , lookup whether or not sucess
    ret = fev_add_event(fev, fd, FEV_READ);
    FTU_ASSERT_EQUAL_INT(-2, ret);

    fev_destroy(fev);
    close(fd);
}

static fev_state* g_fev = NULL;
static int start = 0;
static int end = 0;
static fev_listen_info* fli;

static void test_accept(fev_state* fev, int fd, void* ud)
{
    FTU_ASSERT_EXPRESS(g_fev==fev);
    FTU_ASSERT_GREATER_THAN_INT(0, fd);
    close(fd);
}

static void* test_listener(void* arg)
{
    printf("test listener thread startup\n");
    g_fev = fev_create(1024);
    fli = fev_add_listener(g_fev, 17759, test_accept, NULL);
    FTU_ASSERT_EXPRESS(fli!=NULL);

    printf("wait for poll\n");
    start = 1;
    int process = fev_poll(g_fev, -1);
    FTU_ASSERT_EQUAL_INT(1, process);

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
    pthread_create(&tid, 0, test_listener, NULL);

    while(1) {
        sleep(1);   // wait for fev create completed
        if( start ) break;
    }

    int conn_fd = fnet_conn("127.0.0.1", 17759, 1);
    FTU_ASSERT_GREATER_THAN_INT(0, conn_fd);

    pthread_join(tid, NULL);

    close(conn_fd);
}

static void buff_read(fev_state* fev, fev_buff* evbuff, void* arg)
{
    FTU_ASSERT_EXPRESS(fev==g_fev);

    int buff_read_len = fevbuff_get_bufflen(evbuff, FEVBUFF_TYPE_READ);
    FTU_ASSERT_GREATER_THAN_INT(0, buff_read_len);

    int buff_read_used = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_READ);
    FTU_ASSERT_EQUAL_INT(0, buff_read_used);   

    char read_buf[20];
    memset(read_buf, 0, 20);
    int read_size = fevbuff_read(evbuff, read_buf, 20);
    if( read_size > 0 ) {
        buff_read_used = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_READ);
        FTU_ASSERT_EQUAL_INT(buff_read_used, read_size);
        printf("read size=%d, read_str=%s\n", read_size, read_buf);

        char compare_str[20];
        memset(compare_str, 0, 20);
        snprintf(compare_str, read_size, "hello final");
        FTU_ASSERT_EQUAL_CHAR(compare_str, read_buf);
        
        int pop_len = fevbuff_pop(evbuff, read_size);
        FTU_ASSERT_EQUAL_INT(read_size, pop_len);

        char* write_str = "hi final";
        int write_len = fevbuff_write(evbuff, write_str, 9);
        FTU_ASSERT_EQUAL_INT(9, write_len);
    }
    else{
        //error happened
        printf("error happened haha\n");
    }
}

static void buff_error(fev_state* fev, fev_buff* evbuff, void* arg)
{
    printf("evbuff error\n");
    int fd = fevbuff_destroy(evbuff);
    FTU_ASSERT_GREATER_THAN_INT(0, fd);
    close(fd);

    end = 1;
}

static void fake_accept(fev_state* fev, int fd, void* ud)
{
    fev_buff* evbuff = fevbuff_new(fev, fd, buff_read, buff_error, NULL);
    FTU_ASSERT_EXPRESS(evbuff!=NULL);

    int test_fd = fevbuff_get_fd(evbuff);
    FTU_ASSERT_EQUAL_INT(fd, test_fd);

    void* test_arg = fevbuff_get_arg(evbuff);
    FTU_ASSERT_EXPRESS(test_arg==NULL);

    int buff_read_len = fevbuff_get_bufflen(evbuff, FEVBUFF_TYPE_READ);
    FTU_ASSERT_EQUAL_INT((1024*4), buff_read_len);

    int buff_write_len = fevbuff_get_bufflen(evbuff, FEVBUFF_TYPE_WRITE);
    FTU_ASSERT_EQUAL_INT((1024*4), buff_write_len);

    int buff_read_used = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_READ);
    FTU_ASSERT_EQUAL_INT(0, buff_read_used);

    int buff_write_used = fevbuff_get_usedlen(evbuff, FEVBUFF_TYPE_WRITE);
    FTU_ASSERT_EQUAL_INT(0, buff_write_used);
}

static void* fake_listener(void* arg)
{
    g_fev = fev_create(1024);
    fli = fev_add_listener(g_fev, 17759, fake_accept, NULL);
    FTU_ASSERT_EXPRESS(fli!=NULL);

    printf("wait for poll\n");
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
    FTU_ASSERT_GREATER_THAN_INT(0, conn_fd);

    char* send_str = "hello final";
    int send_num = fnet_send_safe(conn_fd, send_str, strlen(send_str)+1);
    FTU_ASSERT_EQUAL_INT(12, send_num);

    // recv a string
    char recv_buf[20];
    memset(recv_buf, 0, 20);
    int recv_size = fnet_recv(conn_fd, recv_buf, 20);
    FTU_ASSERT_EQUAL_INT(9, recv_size);
    printf("main recv str=%s\n", recv_buf);
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
    printf("accept sucessful\n");
    //close(fd);
}

static void test_for_conn(int fd, conn_arg_t arg)
{
    printf("tid=%lu\n", pthread_self());
    FTU_ASSERT_GREATER_THAN_INT(0, fd);
    FTU_ASSERT_EQUAL_INT(FEV_NIL, fev_get_mask(g_fev, fd));
    close(fd);
    start = 0;
}

static void* fake_listener1(void* arg)
{
    g_fev = fev_create(1024);
    FTU_ASSERT( fev_conn_module_init(g_fev) == 0 );
    fli = fev_add_listener(g_fev, 17759, fake_accept1, NULL);
    FTU_ASSERT_EXPRESS(fli != NULL);

    printf("wait for poll\n");
    start = 1;

    conn_arg_t carg;
    fev_conn(g_fev, "127.0.0.1", 17759, 5000, test_for_conn, carg);

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

    printf("main tid=%lu\n", pthread_self());
    pthread_t tid;
    pthread_create(&tid, 0, fake_listener1, NULL);

    pthread_join(tid, NULL);
}
