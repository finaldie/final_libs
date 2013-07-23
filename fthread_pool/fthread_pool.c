#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "flock.h"
#include "fmbuf.h"
#include "fhash.h"
#include "fthread_pool.h"

#define TH_QUEUE_BUF_SIZE (1024 * 10 * sizeof(th_msg_t))
#define TH_POOL_HASH_SIZE 10

typedef enum {
    TH_TASK = 0,
    TH_RELEASE
} TH_EVENT;

typedef struct {
    int      tid;
    cond_var cond;
    fmbuf*    pbuf;
    void*    parg;
} thread_data;

typedef struct {
    int      ev;
    fth_task pf;
    void*    arg;
} th_msg_t;

static int th_id = 0;
static int max_num = 0;
static int curr_post = 0;
static int curr_num = 0;
static fhash* g_th_pool = NULL;
static thread_data** pth_pool = NULL;

static
void*    base_work(void* arg)
{
    thread_data* th_data = (thread_data*)arg;
    th_msg_t msg;

L:
    cond_wait(&th_data->cond);

    do {
        int ret = fmbuf_pop(th_data->pbuf, &msg, sizeof(th_msg_t));
        if ( ret ) break;

        switch ( msg.ev ) {
            case TH_TASK:
                msg.pf( th_data->parg, msg.arg );
                break;
            case TH_RELEASE:
                return NULL;
        }
    }while(1);

    goto L;
    return NULL;
}

void    fthpool_init(int num)
{
    if ( g_th_pool ) return;
    if ( num <= 0 ) return;

    g_th_pool = fhash_create(TH_POOL_HASH_SIZE);
    pth_pool = (thread_data**)malloc( sizeof(thread_data*) * num );

    max_num = num;
}

int     fthpool_add_thread(void* pri_arg)
{
    thread_data* th_data = malloc(sizeof(thread_data));
    th_data->tid = th_id++;
    th_data->pbuf = fmbuf_create(TH_QUEUE_BUF_SIZE);
    th_data->parg = pri_arg;
    cond_init(&th_data->cond);

    pthread_t t;
    int rc = pthread_create(&t, 0, base_work, th_data);
    if ( 0 != rc ) {
        fmbuf_delete(th_data->pbuf);
        free(th_data);
        return -1;
    }

    fhash_set_int(g_th_pool, th_data->tid, th_data);
    pth_pool[curr_num++] = th_data;

    return th_data->tid;
}

// return 0 : post sucess
// return 1 : post failed .. queue full
int     fthpool_post_task(fth_task pf, void* arg)
{
    if ( !pf ) return 1;

    th_msg_t tmsg;
    tmsg.ev = TH_TASK;
    tmsg.pf = pf;
    tmsg.arg = arg;

    ++curr_post;
    curr_post = curr_post < max_num ? curr_post : 0;
    thread_data* pdata = pth_pool[curr_post];
    if ( fmbuf_push(pdata->pbuf, &tmsg, sizeof(th_msg_t)) )
        return 1;

    cond_wakeup(&pdata->cond);

    return 0;
}
