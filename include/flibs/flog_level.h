#ifndef FLOG_LEVEL_H
#define FLOG_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

struct flog_file_t;

// fLog Levels
#define FLOG_LEVEL_TRACE 0
#define FLOG_LEVEL_DEBUG 1
#define FLOG_LEVEL_INFO  2
#define FLOG_LEVEL_WARN  3
#define FLOG_LEVEL_ERROR 4
#define FLOG_LEVEL_FATAL 5

/**
 * @brief Set log level for a logger
 *
 * @param level  log level
 * @return       previous log level
 */
int flog_set_level(struct flog_file_t*, int level);

/**
 * @brief Get loggers' log level
 *
 * @return       current log level
 */
int flog_get_level(struct flog_file_t*);

int flog_is_trace_enabled(struct flog_file_t*);
int flog_is_debug_enabled(struct flog_file_t*);
int flog_is_info_enabled (struct flog_file_t*);
int flog_is_warn_enabled (struct flog_file_t*);
int flog_is_error_enabled(struct flog_file_t*);
int flog_is_fatal_enabled(struct flog_file_t*);

#ifdef __cplusplus
}
#endif

#endif

