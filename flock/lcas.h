#ifndef _L_CAS_H_
#define _L_CAS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define cas(_a, _o, _n)                                     \
({	__typeof__(_o) __o = _o;                                \
   	__asm__ __volatile__(                                   \
		"lock cmpxchg %3,%1"                                \
		: "=a" (__o), "=m" (*(volatile unsigned int *)(_a)) \
		:  "0" (__o), "r" (_n) );                           \
		__o;                                                \
})

#define ATOMIC_SMP_LOCK "lock ; "
typedef struct { volatile int counter; } atomic_t;

static __inline__ void atomic_inc(atomic_t *v)
{
	__asm__ __volatile__(
		ATOMIC_SMP_LOCK "incl %0"
		:"=m" (v->counter)
		:"m" (v->counter));
}

static __inline__ void atomic_dec(atomic_t *v)
{
	__asm__ __volatile__(
		ATOMIC_SMP_LOCK "decl %0"
		:"=m" (v->counter)
		:"m" (v->counter));
}

static __inline__ long
atomic_add(long *value, long add)
{
	long old;
	
	__asm__ volatile (
		ATOMIC_SMP_LOCK 
 		" xadd  %2, %1; "
		: "=a" (old) : "m" (*value), "a" (add) : "cc", "memory");

	return old;
}

#ifdef __cplusplus
}
#endif

#endif
