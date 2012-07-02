#ifndef _L_AIO_WRITE_H_
#define    _L_AIO_WRITE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <aio.h>

typedef struct aiocb acb;
// when write complete call back with write len
typedef int (*pfunc_wc)(void*, int);

typedef struct _laio_w{
    acb      _acb;
    pfunc_wc pwc;
    void*    data;
}laio_w;


void    laio_init(laio_w*, pfunc_wc, void*);
void    laio_write(laio_w*, int fd, void* buff, int len);

#ifdef __cplusplus
}
#endif

#endif
