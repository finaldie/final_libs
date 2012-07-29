/** ==========================================================================
 *       Filename:  compiler.h
 *
 *    Description:  Some compiler marcos for compiler
 *
 *        Version:  1.0
 *        Created:  07/29/2012 21:05:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 * =========================================================================*/

#if ((__GNUC__ == 2) && ( __GNUC_MINOR__ >= 96 )) || ( __GNUC__ >= 3 )
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x)   (x)
#define unlikely(x) (x)
#endif
