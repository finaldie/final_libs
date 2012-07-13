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

typedef enum {
    LOG_SYNC_MODE,
    LOG_ASYNC_MODE
}LOG_MODE;

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
 *  @param file_name
 *  @param log - log message
 *  @return 0 - success
 *  @return 1 - failed
 */
size_t log_file_write(log_file_t*, const char* log, size_t len);

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

#ifdef __cplusplus
}
#endif

#endif
