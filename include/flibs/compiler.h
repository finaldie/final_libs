#ifndef FLIBS_COMPILER_H
#define FLIBS_COMPILER_H

struct fcompiler;

#if ((__GNUC__ == 2) && ( __GNUC_MINOR__ >= 96 )) || ( __GNUC__ >= 3 ) || defined __clang__
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x)   (x)
#define unlikely(x) (x)
#endif

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 1) || defined __clang__
#define FMEM_BARRIER() __sync_synchronize()
#else
#define FMEM_BARRIER()
#endif

#endif

