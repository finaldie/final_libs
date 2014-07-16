/**
 *  Light Weight Memory Buffer, which can be used for Ring-Buffer or A Simple
 *  Array
 */

#ifndef MBUF_H_FINAL
#define MBUF_H_FINAL

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define FMBUF_SEEK_LEFT  0
#define FMBUF_SEEK_RIGHT 1

typedef struct _mbuf fmbuf;

fmbuf*  fmbuf_create(size_t size);

// create a fmbuf which be populated by 'fill'
fmbuf*  fmbuf_create1(size_t size, int fill);
void    fmbuf_delete(fmbuf* mbuf);

// public interface
size_t  fmbuf_size(fmbuf* mbuf);
size_t  fmbuf_free(fmbuf* mbuf);
size_t  fmbuf_used(fmbuf* mbuf);
void    fmbuf_clear(fmbuf* mbuf);
void*   fmbuf_head(fmbuf* mbuf);
void*   fmbuf_tail(fmbuf* mbuf);
fmbuf*  fmbuf_realloc(fmbuf* mbuf, size_t size);

// ----------------As a Ring Buff------------------------

// push data into mbuf
//
// @param pbuf  mbuf
// @param data  the data user want to push to mbuf
// @param size  the size of data
//
// @return
//   - **0**: success
//   - **1**: failure
int     fmbuf_push(fmbuf* mbuf, const void* data, size_t size);

// pop data from mbuf
//
// @param pbuf  mbuf
// @param buf   the data will be copied to the user *buffer*
// @param size  the size of the user buf
//
// @note if the buf is NULL, the data will still be poped up
//
// @return
//   - **0**: success
//   - **1**: failure: the mbuf size < user expected size
int     fmbuf_pop(fmbuf* mbuf, void* buf, size_t size);

// pop data from mbuf, but actually almost 99% pop action won't involve the
// data copy, it just return the data pointer of the real data
//
// @note user *MUST ALWAYS* use the return pointer as the data pointer
//
// @param pbuf  mbuf
// @param buf   the data will be copied to the user *buffer* (99% it won't copy)
// @param size  the size of the user buf
//
// @return      the real data pointer
void*   fmbuf_vpop(fmbuf* mbuf, void* buf, size_t size);

// get the data pointer of raw data, and it won't consume the data, so user
// still can pop the data later
//
// @note user *MUST ALWAYS* use the return pointer as the data pointer
//
// @param pbuf  mbuf
// @param buf   the data will be copied to the user *buffer* (99% it won't copy)
// @param size  the size of the user buf
//
// @return      the real data pointer
void*   fmbuf_rawget(fmbuf* mbuf, void* buf, size_t size);

//-----------------As a Array-----------------------------

// Allocate a specific size of buffer from the mbuf
//
// @param pbuf  mbuf
// @param size  the size user want to allocate
//
// @return
//   - the begin location of the buffer
//   - NULL when there is no enough space
void*   fmbuf_alloc(fmbuf* mbuf, size_t size);

// Move the header pointer of the mbuf
//
// @param pbuf      mbuf
// @param bytes     the distance of user want to move,
// @param direction - FMBUF_SEEK_LEFT: move left the header
//                  - FMBUF_SEEK_LEFT: move right the header
//
// @return void
void    fmbuf_head_seek(fmbuf* mbuf, size_t offset, int direction);

// Move the tailer pointer of the mbuf
//
// @param pbuf      mbuf
// @param offset    the distance of user want to move,
// @param direction - FMBUF_SEEK_LEFT: move left the tailer
//                  - FMBUF_SEEK_LEFT: move right the tailer
//
// @return void
void    fmbuf_tail_seek(fmbuf* mbuf, size_t offset, int direction);

// Reset the header pointer to the begin of the mbuf, then move the data and
// tailer pointer as well
//
// @param mbuf
// @return void
void    fmbuf_rewind(fmbuf* mbuf);

// return the how many free space before the header pointer
//
// @param mbuf
// @return void
size_t  fmbuf_head_free(fmbuf* mbuf);

// return the how many free space after the tailer pointer
//
// @param mbuf
// @return void
size_t  fmbuf_tail_free(fmbuf* mbuf);

#ifdef __cplusplus
}
#endif

#endif
