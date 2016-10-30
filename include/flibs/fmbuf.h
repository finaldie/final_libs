/**
 *  Light Weight Memory Buffer, which can be used for Ring-Buffer or A Simple
 *  Array
 */

#ifndef FMBUF_H_FINAL
#define FMBUF_H_FINAL

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define FMBUF_SEEK_LEFT  0
#define FMBUF_SEEK_RIGHT 1

typedef struct _mbuf fmbuf;

/**
 * create a fmbuf
 *
 * @param size  the buffer size user want to create
 * @return
 *   - the pointer of fmbuf
 *   - NULL if create failure when there is no available memory
 *
 * @note if the size is `0`, it will create a empty fmbuf with size is 0
 */
fmbuf*  fmbuf_create(size_t size);

/**
 * destroy a fmbuf
 *
 * @param mbuf  pointer of fmbuf
 * @return void
 */
void    fmbuf_delete(fmbuf* mbuf);

/**
 * return the total size of fmbuf
 *
 * @param mbuf  pointer of fmbuf
 * @return the total size of fmbuf
 */
size_t  fmbuf_size(fmbuf* mbuf);

/**
 * return how many bytes left of the fmbuf
 *
 * @param mbuf  pointer of fmbuf
 * @return the unused size of fmbuf
 */
size_t  fmbuf_free(fmbuf* mbuf);

/**
 * return how many bytes used of the fmbuf
 *
 * @param mbuf  pointer of fmbuf
 * @return the used size of fmbuf
 */
size_t  fmbuf_used(fmbuf* mbuf);

/**
 * reset a fmbuf, and it will be a empty fmbuf after that
 *
 * @param mbuf  pointer of fmbuf
 * @return void
 */
void    fmbuf_clear(fmbuf* mbuf);

/**
 * return the header pointer of the fmbuf
 *
 * @param mbuf  pointer of fmbuf
 * @return the header pointer of the fmbuf
 */
void*   fmbuf_head(fmbuf* mbuf);

/**
 * return the tailer pointer of the fmbuf
 *
 * @param mbuf  pointer of fmbuf
 * @return the tailer pointer of the fmbuf
 */
void*   fmbuf_tail(fmbuf* mbuf);

/**
 * resize the fmbuf
 *
 * @param mbuf  pointer of fmbuf
 * @param size  the new size user want to be
 *
 * @return the new fmbuf with the new size
 *
 * @note user *MUST ALWAYS* use the return pointer as the new fmbuf, the old
 *       pointer may not available
 */
fmbuf*  fmbuf_realloc(fmbuf* mbuf, size_t size);

// ----------------As a Ring Buff------------------------

/**
 * push data into mbuf
 *
 * @param mbuf  pointer of fmbuf
 * @param data  the data user want to push to mbuf
 * @param size  the size of data
 *
 * @return
 *   - **0**: success
 *   - **1**: failure
 */
int     fmbuf_push(fmbuf* mbuf, const void* data, size_t size);

/**
 * pop data from mbuf
 *
 * @param mbuf  pointer of fmbuf
 * @param buf   the data will be copied to the user *buffer*
 * @param size  the size of the user buf
 *
 * @note if the buf is NULL, the data will still be poped up
 *
 * @return
 *   - **0**: success
 *   - **1**: failure: the mbuf size < user expected size
 */
int     fmbuf_pop(fmbuf* mbuf, void* buf, size_t size);

/**
 * pop data from mbuf, but actually almost 99% pop action won't involve the
 * data copy, it just return the data pointer of the real data
 *
 * @note user *MUST ALWAYS* use the return pointer as the data pointer
 *
 * @param mbuf  pointer of fmbuf
 * @param buf   the data will be copied to the user *buffer* (99% it won't copy)
 * @param size  the size of the user buf
 *
 * @return      the real data pointer
 */
void*   fmbuf_vpop(fmbuf* mbuf, void* buf, size_t size);

/**
 * get the data pointer of raw data, and it won't consume the data, so user
 * still can pop the data later
 *
 * @note user *MUST ALWAYS* use the return pointer as the data pointer
 *
 * @param mbuf  pointer of fmbuf
 * @param buf   the data will be copied to the user *buffer* (99% it won't copy)
 * @param size  the size of the user buf
 *
 * @return      the real data pointer
 */
void*   fmbuf_rawget(fmbuf* mbuf, void* buf, size_t size);

//-----------------As a Array-----------------------------

/**
 * Allocate a specific size of buffer from the mbuf
 *
 * @param mbuf  pointer of fmbuf
 * @param size  the size user want to allocate
 *
 * @return
 *   - the begin location of the buffer
 *   - NULL when there is no enough space
 */
void*   fmbuf_alloc(fmbuf* mbuf, size_t size);

/**
 * Move the header pointer of the mbuf
 *
 * @param mbuf      pointer of fmbuf
 * @param offset    the distance of user want to move,
 * @param direction
 *                  - FMBUF_SEEK_LEFT: move left to the header
 *                  - FMBUF_SEEK_RIGHT: move right to the header
 *
 * @return void
 */
void    fmbuf_head_seek(fmbuf* mbuf, size_t offset, int direction);

/**
 * Move the tailer pointer of the fmbuf
 *
 * @param mbuf      pointer of fmbuf
 * @param offset    the distance of user want to move,
 * @param direction
 *                  - FMBUF_SEEK_LEFT: move left to the tailer
 *                  - FMBUF_SEEK_RIGHT: move right to the tailer
 *
 * @return void
 */
void    fmbuf_tail_seek(fmbuf* mbuf, size_t offset, int direction);

/**
 * Reset the header pointer to the begin of the mbuf, then move the data and
 * tailer pointer as well
 *
 * @param mbuf  pointer of fmbuf
 * @return void
 *
 * @note it only can be used when the fmbuf as a array
 */
void    fmbuf_rewind(fmbuf* mbuf);

/**
 * return the how many free space before the header pointer
 *
 * @param mbuf  pointer of fmbuf
 * @return void
 */
size_t  fmbuf_head_free(fmbuf* mbuf);

/**
 * return the how many free space after the tailer pointer
 *
 * @param mbuf  pointer of fmbuf
 * @return void
 */
size_t  fmbuf_tail_free(fmbuf* mbuf);

#ifdef __cplusplus
}
#endif

#endif

