/******************************************************************************
 * Base info: Created by Yuzhang Hu
 * modify: 06/20/2012
 * Description: Asynchronous logging system
 *   The base idea of this system is using the thread cache to buffer log
 *   messages, and there is one background thread to fetch log messages.
 *   So it try the best to remove locks, which will be influence performance.
 *   It could be very fast, simple and strong. have a fun :)
 *****************************************************************************/

#ifndef FLOG_SYSTEM_H
#define FLOG_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "flog_level.h"
#include "flog_helpers.h"

typedef enum {
    FLOG_SYNC_MODE,
    FLOG_ASYNC_MODE
} FLOG_MODE;

typedef enum {
    FLOG_EVENT_ERROR_WRITE,
    FLOG_EVENT_BUFF_FULL,
    FLOG_EVENT_USER_BUFFER_RELEASED
} FLOG_EVENT;

typedef void (*flog_event_func)(FLOG_EVENT);
typedef struct flog_file_t flog_file_t;

/**
 *  @brief Create Logger
 *
 *  @param filename
 *
 *  @return a pointer of log_file structure
 */
flog_file_t* flog_create(const char* filename);

/**
 *  @brief Destroy Logger
 *
 *  @param logger   which will be destroy
 *
 *  @return         void
 */
void flog_destroy(flog_file_t* logger);

/**
 *  @brief Write log
 *
 *  @param file_sig the signature of log message
 *  @param sig_len  length of signature
 *  @param log      log message
 *  @param len      length of message
 *
 *  @return
 *                  - 0: success
 *                  - 1: failed
 */
size_t flog_file_write(flog_file_t*, const char* file_sig, size_t sig_len,
                        const char* log, size_t len);

/**
 *  @brief Write log with format
 *
 *  @param file_sig the signature of log message
 *  @param sig_len  length of signature
 *  @param fmt      format string
 *  @param ...      dynamic args for format
 *
 *  @return         void
 */
void flog_file_write_f(flog_file_t*, const char* file_sig, size_t sig_len,
                        const char* fmt, ...);

/**
 *  @brief Set log mode
 *
 *  @param mode
 *                  - LOG_SYNC_MODE: set synchronization mode
 *                  - LOG_ASYNC_MODE: set asynchronization mode
 *
 *  @return         mode before setting
 */
FLOG_MODE flog_set_mode(FLOG_MODE mode);

/**
 *  @brief Set file roll size, when greater than the given size, log system
 *      will roll a new file to continue writing messages, default value is 2G
 *
 *  @param size     set max size for rolling
 *
 *  @return         void
 */
void flog_set_roll_size(size_t size);

/**
 *  @brief Set max flush interval, unit msec
 *
 *  @param sec      after given time, it will force to flush, default value is 0
 *
 *  @return         void
 */
void flog_set_flush_interval(size_t sec);

/**
 *  @brief Set buffer size for per user thread, should set before async writing
 *    The default buffer size per thread is 10M, call this interface if you
 *    want to change it
 *
 *  @param size     buffer size per thread
 *
 *  @return         void
 */
void flog_set_buffer_size(size_t size);

/**
 *  @brief Get buffer size of per user thread
 *
 *  @return         buffer size
 */
size_t flog_get_buffer_size();

/**
 *  @brief Register a callback function for notifying user some important status
 *
 *  @param pfunc    user callback function
 *
 *  @return         void
 */
void flog_register_event_callback(flog_event_func pfunc);

#ifdef __cplusplus
}
#endif

#endif
