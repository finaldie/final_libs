#ifndef FLOCK_H
#define FLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct flock_cond_t {
    unsigned long   _count;

#if __WORDSIZE == 32
    int             _padding;
#endif

    pthread_mutex_t _mutex;
    pthread_cond_t  _cond;
} flock_cond_t;

void flock_cond_init(flock_cond_t*);
void flock_cond_wait(flock_cond_t*);
void flock_cond_signal(flock_cond_t*);
void flock_cond_destroy(flock_cond_t*);

#ifdef __cplusplus
}
#endif

#endif

