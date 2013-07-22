// create by final
// desc: wrap the mutex

#ifndef _LMUTEX_H_FINAL_
#define _LMUTEX_H_FINAL_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
typedef struct _cond_var {
    unsigned int    _count;
    pthread_mutex_t _mutex;
    pthread_cond_t  _cond;
} cond_var;

void    cond_init(cond_var*);
void    cond_wait(cond_var*);
void    cond_wakeup(cond_var*);
void    cond_del(cond_var*);

#ifdef __cplusplus
}
#endif

#endif 

