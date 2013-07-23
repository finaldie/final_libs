#ifndef MBUF_H_FINAL
#define MBUF_H_FINAL

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mbuf fmbuf;

fmbuf*  fmbuf_create(size_t size);
void    fmbuf_delete(fmbuf* pbuf);

// public interface 
int     fmbuf_size(fmbuf* pbuf);
int     fmbuf_used(fmbuf* pbuf);
void    fmbuf_clear(fmbuf* pbuf);
void*   fmbuf_get_head(fmbuf* pbuf);
void*   fmbuf_get_tail(fmbuf* pbuf);

// ----------------As a Loop Buff------------------------
//push data into mbuf : if tail free space not enough that continue write from start
//return 0/1 : 0-> sucess 1->fail
int     fmbuf_push(fmbuf* pbuf, const void* data, size_t size);
int     fmbuf_pop(fmbuf* pbuf, void* data, size_t size);
void*   fmbuf_vpop(fmbuf* pbuf, void* data, size_t size);
void*   fmbuf_getraw(fmbuf* pbuf, void* data, size_t size);
void    fmbuf_head_move(fmbuf* pbuf, size_t size);
int     fmbuf_free(fmbuf* pbuf);

//-----------------As a Array-----------------------------
//alloc a buf from mbuf
//return NULL : alloc failed
int     fmbuf_head_free(fmbuf* pbuf);
int     fmbuf_tail_free(fmbuf* pbuf);
int     fmbuf_total_free(fmbuf* pbuf);

void*   fmbuf_alloc(fmbuf* pbuf, size_t size);
void    fmbuf_head_seek(fmbuf* pbuf, int offset);
void    fmbuf_tail_seek(fmbuf* pbuf, int offset);
void    fmbuf_rewind(fmbuf* pbuf);
fmbuf*  fmbuf_realloc(fmbuf* pbuf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
