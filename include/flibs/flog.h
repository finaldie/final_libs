/******************************************************************************
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
#include <stdarg.h>
#include <time.h>

#include "flibs/flog_level.h"
#include "flibs/flog_helpers.h"

/**
 * @brief flog working mode: sync or async
 */
typedef enum {
    FLOG_SYNC_MODE = 0,
    FLOG_ASYNC_MODE
} flog_mode_t;

/**
 * @brief flog event types
 */
typedef enum {
    // error metrics
    FLOG_EVENT_ERROR_WRITE = 0,         // write error
    FLOG_EVENT_ERROR_ASYNC_PUSH,        // push data failed in async write
    FLOG_EVENT_ERROR_ASYNC_POP,         // pop data failed in async write
    FLOG_EVENT_TRUNCATED,               // log was been truncated
    FLOG_EVENT_BUFFER_FULL,             // per-thread log buffer full

    // normal notice metrics
    FLOG_EVENT_USER_BUFFER_RELEASED,    // user logger thread quit gracefully
} flog_event_t;

/**
 * @brief flog notification callback, handle exceptions here
 */
typedef void (*flog_event_func)(flog_event_t);

/**
 * @brief flog handler
 */
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
 *  @param log      log message
 *  @param len      length of message
 *
 *  @return
 *                  - 0: success
 *                  - 1: failed
 */
size_t flog_write(flog_file_t*, const char* log, size_t len);

/**
 *  @brief Write log with format
 *
 *  @param fmt      format string
 *  @param ...      dynamic args for format
 *
 *  @return         void
 */
void flog_writef(flog_file_t*, const char* fmt, ...);

/**
 *  @brief Write log with format by a va_list
 *
 *  @param fmt      format string
 *  @param ap       dynamic arg list using by the fmt
 *
 *  @return         actual writen size
 */
void flog_vwritef(flog_file_t*, const char* fmt, va_list ap);

/**
 * @brief Set log cookie, user set up once, then every log will include this
 *        cookie string
 * @note  Cookie string is per-thread, and the max length of cookie is 256 bytes
 *
 * @param fmt       string format
 * @return          void
 */
void flog_set_cookie(const char* fmt, ...);

/**
 * @brief Set log cookie, user set up once, then every log will include this
 *        cookie string
 * @note  Cookie string is per-thread, and the max length of cookie is 256 bytes
 *
 * @param fmt       string format
 * @param ap        dynamic arg list using by fmt
 * @return          void
 */
void flog_vset_cookie(const char* fmt, va_list ap);

/**
 * @brief Clear the cookie string
 * @note  Clear action only impact the current thread cookie data
 *
 * @return          void
 */
void flog_clear_cookie();

/**
 *  @brief Set log mode
 *
 *  @param mode
 *                  - LOG_SYNC_MODE: set synchronization mode
 *                  - LOG_ASYNC_MODE: set asynchronization mode
 *
 *  @return         mode before setting
 */
flog_mode_t flog_set_mode(flog_mode_t mode);

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
 *  @brief Set max flush interval, unit second
 *
 *  @param sec      after given time, it will force to flush, default value is 0
 *
 *  @return         void
 */
void flog_set_flush_interval(time_t sec);

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

