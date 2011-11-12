//base info: create by hyz
/*effect: thread cache mempool
*
*
*/


#ifndef _FMEM_POOL_H_
#define _FMEM_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif

void* f_alloc(size_t size);
void  f_free(void* ptr);
void* f_realloc(void* ptr, size_t size);
void* f_calloc(size_t num, size_t size);

#ifdef __cplusplus
}
#endif

#endif
