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

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_NOTICE 2
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_CRIT  5
#define LOG_LEVEL_ALERT 6
#define LOG_LEVEL_EMERG 7

#define FLOG_DEBUG(log_handler, fmt, args...)  if( is_debug_enable()  ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_DEBUG,  fmt, ##args); }
#define FLOG_INFO(log_handler, fmt, args...)   if( is_info_enable()   ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_INFO,   fmt, ##args); }
#define FLOG_NOTICE(log_handler, fmt, args...) if( is_notice_enable() ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_NOTICE, fmt, ##args); }
#define FLOG_WARN(log_handler, fmt, args...)   if( is_warn_enable()   ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_WARN,   fmt, ##args); }
#define FLOG_ERROR(log_handler, fmt, args...)  if( is_error_enable()  ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_ERROR,  fmt, ##args); }
#define FLOG_CRIT(log_handler, fmt, args...)   if( is_crit_enable()   ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_CRIT,   fmt, ##args); }
#define FLOG_ALERT(log_handler, fmt, args...)  if( is_alert_enable()  ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_ALERT,  fmt, ##args); }
#define FLOG_EMERG(log_handler, fmt, args...)  if( is_emerg_enable()  ) { flog(log_handler, __FILE__, __LINE__, __func__, LOG_LEVEL_EMERG,  fmt, ##args); }

int flog_create(const char* file_name);
int flog(int log_handler, const char* src_filename, int lineno,
        const char* func_name, int level, const char* fmt, ...);
int flog_set_level(int level);
int flog_get_level();

inline int is_debug_enable();
inline int is_info_enable();
inline int is_notice_enable();
inline int is_warn_enable();
inline int is_error_enable();
inline int is_crit_enable();
inline int is_alert_enable();
inline int is_emerg_enable();

#ifdef __cplusplus
}
#endif

#endif
