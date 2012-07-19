//base info: create by hyz
/*effect: log system include file
*
*
*/

#ifndef _LOG_INCLUDE_H_
#define _LOG_INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* flogger;

// LOG MODE
typedef enum {
    FLOG_SYNC_MODE,
    FLOG_ASYNC_MODE
}FLOG_MODE;

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5

#define FLOG_TRACE(log_handler, fmt, args...)  if( is_trace_enable()  ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_TRACE, fmt, ##args); }
#define FLOG_DEBUG(log_handler, fmt, args...)  if( is_debug_enable()  ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_DEBUG, fmt, ##args); }
#define FLOG_INFO(log_handler, fmt, args...)   if( is_info_enable()   ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_INFO,  fmt, ##args); }
#define FLOG_WARN(log_handler, fmt, args...)   if( is_warn_enable()   ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_WARN,  fmt, ##args); }
#define FLOG_ERROR(log_handler, fmt, args...)  if( is_error_enable()  ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_ERROR, fmt, ##args); }
#define FLOG_FATAL(log_handler, fmt, args...)  if( is_fatal_enable()  ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_FATAL, fmt, ##args); }

flogger* flog_create(const char* file_name);
void     flog_destroy(flogger* fl);
int      flog(flogger*, const char* src_filename, int lineno,
            const char* func_name, int level, const char* fmt, ...);
int      flog_set_level(int level);
int      flog_get_level();
void     flog_set_mode(FLOG_MODE);
void     flog_set_roll_size(size_t size);
void     flog_set_flush_interval(size_t sec);

int is_trace_enable();
int is_debug_enable();
int is_info_enable();
int is_warn_enable();
int is_error_enable();
int is_fatal_enable();

#ifdef __cplusplus
}
#endif

#endif
