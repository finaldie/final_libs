#ifndef FLIBS_COMPILER_H
#define FLIBS_COMPILER_H

#if ((__GNUC__ == 2) && ( __GNUC_MINOR__ >= 96 )) || ( __GNUC__ >= 3 )
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x)   (x)
#define unlikely(x) (x)
#endif

#endif

