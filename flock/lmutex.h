// create by final
// desc: wrap the mutex 

#ifndef _LMUTEX_H_FINAL_
#define _LMUTEX_H_FINAL_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <bits/pthreadtypes.h>

typedef unsigned int _uint;

typedef struct _cond_var
{
	_uint 			_count;
	pthread_mutex_t _mutex;
	pthread_cond_t  _cond;
}cond_var;

void	cond_init(cond_var*);
void	cond_wait(cond_var*);
void	cond_wakeup(cond_var*);
void	cond_del(cond_var*);

typedef struct 
{
	pthread_mutex_t _mutex;
}mutex_var;

void	mutex_init(mutex_var*);
void	mutex_lock(mutex_var*);
void	mutex_unlock(mutex_var*);
void	mutex_del(mutex_var*);

typedef struct
{
	pthread_spinlock_t	_lock;
}spin_var;

void	spin_init(spin_var*);
void	spin_lock(spin_var*);
void	spin_unlock(spin_var*);
void	spin_del(spin_var*);

#ifdef __cplusplus
}
#endif

#endif 

