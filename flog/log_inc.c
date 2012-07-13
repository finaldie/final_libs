#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "log_inc.h"
#include "log.h"

#define LOG_TIME_LEN 22
#define LOG_MAX_LEN  1024

static const char* level_str[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};
/** default set by INFO
 *  use flog_set_level() to change the priority
 */
static int _f_log_level = LOG_LEVEL_INFO;

static
void    log_get_time(char* now_time)
{
    // need buff len = 22
    time_t tm_time = time(NULL);
    struct tm now;
    gmtime_r(&tm_time, &now);
    snprintf(now_time, LOG_TIME_LEN, "[%04d-%02d-%02d %02d:%02d:%02d]",
                (now.tm_year+1900), now.tm_mon+1, now.tm_mday,
                now.tm_hour, now.tm_min, now.tm_sec);
}

inline
flogger* flog_create(const char* file_name)
{
    return (flogger*)log_create(file_name);
}

void flog_destroy(flogger* fl)
{
    log_destroy((log_file_t*)fl);
}

inline
int flog_set_level(int level)
{
    if( level < LOG_LEVEL_TRACE || level > LOG_LEVEL_FATAL )
        return 1; //error
    _f_log_level = level;
    return 0;
}

inline
int    flog_get_level()
{
    return _f_log_level;
}

int    flog(flogger* log_handler, const char* src_filename, int lineno,
        const char* func_name, int level, const char* fmt, ...)
{
    if( !log_handler ) return 1;
    if( level < LOG_LEVEL_TRACE || level > LOG_LEVEL_FATAL )
        return 2;
    if( !fmt ) return 3;

    char log_data[LOG_MAX_LEN];
    char now[LOG_TIME_LEN];
    va_list va;

    log_get_time(now);
    const char* _level_str = level_str[level];
    int head_len = sprintf(log_data, "%s [%s] %s:%s(%d) - ", now, _level_str,
            src_filename, func_name, lineno);

    int write_len = 0;
    int max_len = LOG_MAX_LEN - head_len - 2;
    va_start(va, fmt);
    write_len = vsnprintf(log_data + head_len, max_len, fmt, va);
    va_end(va);

    // if write_len > max_len, that means our log has been truncated
    if ( write_len > max_len ) {
        log_data[LOG_MAX_LEN - 2] = '\n';
        log_data[LOG_MAX_LEN - 1] = '\0';
    } else {
        log_data[head_len + write_len] = '\n';
        log_data[head_len + write_len + 1] = '\0';
    }

    int total_len = head_len + write_len + 1;
    size_t real_write = log_file_write((log_file_t*)log_handler, log_data, total_len);
    if ( real_write < (size_t)(total_len) ) {
        return 4;
    }

    return 0;
}

void flog_set_mode(FLOG_MODE mode)
{
    if ( mode == FLOG_SYNC_MODE ) {
        log_set_mode(LOG_SYNC_MODE);
    } else {
        log_set_mode(LOG_ASYNC_MODE);
    }
}

void flog_set_roll_size(size_t size)
{
    log_set_roll_size(size);
}

void flog_set_flush_interval(size_t sec)
{
    log_set_flush_interval(sec);
}

inline int is_trace_enable()  { return LOG_LEVEL_TRACE  >= flog_get_level(); }
inline int is_debug_enable()  { return LOG_LEVEL_DEBUG  >= flog_get_level(); }
inline int is_info_enable()   { return LOG_LEVEL_INFO   >= flog_get_level(); }
inline int is_warn_enable()   { return LOG_LEVEL_WARN   >= flog_get_level(); }
inline int is_error_enable()  { return LOG_LEVEL_ERROR  >= flog_get_level(); }
inline int is_fatal_enable()  { return LOG_LEVEL_FATAL  >= flog_get_level(); }
