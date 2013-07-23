#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*fth_task)(void* thread_data, void* arg);

/* new version use the follow methods */
void    fthpool_init(int num);
// the arg is the every task arg
int     fthpool_post_task(fth_task, void* arg);
// the thread_data arg is this thread fixed arg every callback will push it
int     fthpool_add_thread(void* thread_data);
int     fthpool_del_thread(int tid);

#ifdef __cplusplus
}
#endif

#endif
