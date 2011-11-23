#ifndef	MBUF_H_FINAL
#define MBUF_H_FINAL

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mbuf mbuf;

mbuf*	create_mbuf(size_t size);
void	delete_mbuf(mbuf* pbuf);

int		mbuf_used(mbuf* pbuf);
void	mbuf_clear(mbuf* pbuf);
int		mbuf_total_free(mbuf* pbuf);
int		mbuf_tail_free(mbuf* pbuf);
int		mbuf_head_free(mbuf* pbuf);
void*	mbuf_get_head(mbuf* pbuf);
void*	mbuf_get_tail(mbuf* pbuf);
int     mbuf_size(mbuf* pbuf);

//push data into mbuf : if tail free space not enough that continue write from start
//return 0/1 : 0-> sucess 1->fail
int		mbuf_push(mbuf* pbuf, const void* data, size_t size);
int		mbuf_pop(mbuf* pbuf, void* data, size_t size);
void*	mbuf_vpop(mbuf* pbuf, void* data, size_t size);
void*	mbuf_getraw(mbuf* pbuf, void* data, size_t size);
void	mbuf_tail_move(mbuf* pbuf, size_t size);

//------------------------------------------------------------------
//alloc a buf from mbuf
//return NULL : alloc failed
void*	mbuf_alloc(mbuf* pbuf, size_t size);
void	mbuf_head_seek(mbuf* pbuf, int offset);
void	mbuf_tail_seek(mbuf* pbuf, int offset);
void	mbuf_rewind(mbuf* pbuf);
mbuf*	mbuf_realloc(mbuf* pbuf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
