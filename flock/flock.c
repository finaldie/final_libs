#include <stdio.h>
#include <stdlib.h>

#include "flibs/flock.h"

//------------------cond-------------------------------

void    flock_cond_init(flock_cond_t* pcond)
{
    pcond->_count = 0;
    pthread_mutex_init(&pcond->_mutex, NULL);
    pthread_cond_init(&pcond->_cond, NULL);
}

void    flock_cond_wait(flock_cond_t* pcond)
{
    pthread_mutex_lock(&pcond->_mutex);

    while( pcond->_count == 0 )
        pthread_cond_wait(&pcond->_cond, &pcond->_mutex);
    pcond->_count -= 1;

    pthread_mutex_unlock(&pcond->_mutex);
}

void    flock_cond_signal(flock_cond_t* pcond)
{
    pthread_mutex_lock(&pcond->_mutex);

    if( pcond->_count == 0 )
        pthread_cond_signal(&pcond->_cond);
    pcond->_count += 1;

    pthread_mutex_unlock(&pcond->_mutex);
}

void    flock_cond_destroy(flock_cond_t* pcond)
{
    pthread_mutex_destroy(&pcond->_mutex);
    pthread_cond_destroy(&pcond->_cond);
}
