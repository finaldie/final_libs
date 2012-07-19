#include <stdio.h>
#include <stdlib.h>

#include "lmutex.h"

//------------------cond-------------------------------
void    cond_init(cond_var* pcond)
{
    pcond->_count = 0;
    pthread_mutex_init(&pcond->_mutex, NULL);
    pthread_cond_init(&pcond->_cond, NULL);
}

void    cond_wait(cond_var* pcond)
{
    pthread_mutex_lock(&pcond->_mutex);

    while( pcond->_count == 0 )
        pthread_cond_wait(&pcond->_cond, &pcond->_mutex);
    pcond->_count -= 1;

    pthread_mutex_unlock(&pcond->_mutex);
}

void    cond_wakeup(cond_var* pcond)
{
    pthread_mutex_lock(&pcond->_mutex);

    if( pcond->_count == 0 )
        pthread_cond_signal(&pcond->_cond);
    pcond->_count += 1;

    pthread_mutex_unlock(&pcond->_mutex);
}

void    cond_del(cond_var* pcond)
{
    pthread_mutex_destroy(&pcond->_mutex);
    pthread_cond_destroy(&pcond->_cond);
}

//-------------------mutex----------------------------
void    mutex_init(mutex_var* mutex)
{
    pthread_mutex_init(&mutex->_mutex, NULL);
}

inline
void    mutex_lock(mutex_var* mutex)
{
    pthread_mutex_lock(&mutex->_mutex);
}

inline
void    mutex_unlock(mutex_var* mutex)
{
    pthread_mutex_unlock(&mutex->_mutex);
}

void    mutex_del(mutex_var* mutex)
{
    pthread_mutex_destroy(&mutex->_mutex);
}

//-------------------spin-----------------------------
void    spin_init(spin_var* lock)
{
    pthread_spin_init(&lock->_lock, PTHREAD_PROCESS_PRIVATE);
}

inline
void    spin_lock(spin_var* lock)
{
    pthread_spin_lock(&lock->_lock);
}

inline
void    spin_unlock(spin_var* lock)
{
    pthread_spin_unlock(&lock->_lock);
}

void    spin_del(spin_var* lock)
{
    pthread_spin_destroy(&lock->_lock);
}
