//base info: create by hyz
/*effect: log system header file
*
*
*/

#ifndef FLOG_LEVEL_H
#define FLOG_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

// fLog Levels
#define FLOG_LEVEL_TRACE 0
#define FLOG_LEVEL_DEBUG 1
#define FLOG_LEVEL_INFO  2
#define FLOG_LEVEL_WARN  3
#define FLOG_LEVEL_ERROR 4
#define FLOG_LEVEL_FATAL 5

/**
 * @brief Set global log level
 * @note  This is all the flog loggers
 *
 * @param level     log level
 * @return          previous log level
 */
int          flog_set_level(int level);

/**
 * @brief Get global log level
 *
 * @return          current log level
 */
int          flog_get_level();

// private, no need to call them directly
int          flog_is_trace_enabled();
int          flog_is_debug_enabled();
int          flog_is_info_enabled();
int          flog_is_warn_enabled();
int          flog_is_error_enabled();
int          flog_is_fatal_enabled();

#ifdef __cplusplus
}
#endif

#endif
