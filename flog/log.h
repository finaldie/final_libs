/******************************************************************************
 * Base info: Created by Yuzhang Hu
 * modify: 06/20/2012
 * Description: Asynchronous logging system
 *   The base idea of this system is using the thread cache to buffer log
 *   messages, and there is one background thread to fetch log messages.
 *   So it try the best to remove locks, which will be influence performance.
 *   It could be very fast, simple and strong. have a fun :)
 *****************************************************************************/

#ifndef _LOG_SYSTEM_H_
#define _LOG_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdarg.h>

typedef enum {
    LOG_SYNC_MODE,
    LOG_ASYNC_MODE
} LOG_MODE;

typedef enum {
    LOG_EVENT_ERROR_WRITE,
    LOG_EVENT_ERROR_MSG_SIZE,
    LOG_EVENT_BUFF_FULL,
    LOG_EVENT_USER_BUFFER_RELEASED
} LOG_EVENT;

typedef void (*plog_event_func)(LOG_EVENT);
typedef struct _log_file_t log_file_t;

/**
 *  @brief Create Logger
 *  @param filename
 *  @return a pointer of log_file structure
 */
log_file_t* log_create(const char* filename);

/**
 *  @brief Destroy Logger
 *  @param logger - which will be destroy
 *  @return void
 */
void log_destroy(log_file_t* logger);

/**
 *  @brief Write log
 *  @param log - log message
 *  @param len - length of message
 *  @return 0 - success
 *  @return 1 - failed
 */
size_t log_file_write(log_file_t*, const char* log, size_t len);

/**
 *  @brief Write log with format
 *  @param src_filename - filename of writing log
 *  @param func_name
 *  @param lineno - line number
 *  @param fmt - format string
 *  @param ... - dynamic args for format
 *  @return 0 - success
 *  @return 1 - failed
 */
void log_file_write_f(log_file_t*, const char* file_sig, size_t sig_len,
                        const char* fmt, va_list ap);

/**
 *  @brief Set log mode
 *  @param LOG_SYNC_MODE - set synchronization mode
 *  @param LOG_ASYNC_MODE - set asynchronization mode
 *  @return mode before setting
 */
LOG_MODE log_set_mode(LOG_MODE mode);

/**
 *  @brief Set file roll size, when greater than the given size, log system
 *  will roll a new file to continue writing messages
 *  @param size - set max size for rolling
 *  @return void
 */
void log_set_roll_size(size_t size);

/**
 *  @brief Set max flush interval, unit msec
 *  @param sec - after given time, it will force to flush
 *  @return void
 */
void log_set_flush_interval(size_t sec);

/**
 *  @brief Set buffer size for user thread, should set before async writing
 *    The default buffer size per thread is 10M, call this interface if you
 *    want to change it
 *  @param size
 *  @return void
 */
void log_set_buffer_size(size_t size);

/**
 *  @brief Register a callback function for notifying user some important status
 *  @param pfunc - user callback function
 *  @return void
 */
void log_register_event_callback(plog_event_func pfunc);

#ifdef __cplusplus
}
#endif

#endif
