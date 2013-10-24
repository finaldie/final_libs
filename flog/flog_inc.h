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

#include "flog.h"

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5

#define TO_STRX(x) #x
#define TO_STR(x) TO_STRX(x)
#define EXTRACT_STR(s) s, (sizeof(s) - 1)

#define FLOG_TRACE(log_handler, fmt, args...) if( is_trace_enable() ) { log_file_write_f(log_handler, EXTRACT_STR("[TRACE] " __FILE__ ":" TO_STR(__LINE__) " - "), fmt, ##args); }
#define FLOG_DEBUG(log_handler, fmt, args...) if( is_debug_enable() ) { log_file_write_f(log_handler, EXTRACT_STR("[DEBUG] " __FILE__ ":" TO_STR(__LINE__) " - "), fmt, ##args); }
#define FLOG_INFO(log_handler,  fmt, args...) if( is_info_enable()  ) { log_file_write_f(log_handler, EXTRACT_STR("[INFO] "  __FILE__ ":" TO_STR(__LINE__) " - "), fmt, ##args); }
#define FLOG_WARN(log_handler,  fmt, args...) if( is_warn_enable()  ) { log_file_write_f(log_handler, EXTRACT_STR("[WARN] "  __FILE__ ":" TO_STR(__LINE__) " - "), fmt, ##args); }
#define FLOG_ERROR(log_handler, fmt, args...) if( is_error_enable() ) { log_file_write_f(log_handler, EXTRACT_STR("[ERROR] " __FILE__ ":" TO_STR(__LINE__) " - "), fmt, ##args); }
#define FLOG_FATAL(log_handler, fmt, args...) if( is_fatal_enable() ) { log_file_write_f(log_handler, EXTRACT_STR("[FATAL] " __FILE__ ":" TO_STR(__LINE__) " - "), fmt, ##args); }

log_file_t* flog_create(const char* file_name);
void        flog_destroy(log_file_t* fl);
int         flog_set_level(int level);
int         flog_get_level();
void        flog_set_mode(LOG_MODE);
void        flog_set_roll_size(size_t size);
void        flog_set_flush_interval(size_t sec);
void        flog_set_buffer_size(size_t size);
size_t      flog_get_buffer_size();
void        flog_register_event_callback(plog_event_func);

int         is_trace_enable();
int         is_debug_enable();
int         is_info_enable();
int         is_warn_enable();
int         is_error_enable();
int         is_fatal_enable();

#ifdef __cplusplus
}
#endif

#endif
