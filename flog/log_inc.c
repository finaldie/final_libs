#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "log_inc.h"

#ifdef _USE_AIO_
#include "log_aio.h"
#define  FLOG_CREATE()          log_aio_create()
#define  FLOG(filename, buff)   log_aio_write(filename, buff)
#else
#include "log.h"
#define  FLOG_CREATE()          log_create()
#define  FLOG(filename, buff)   log_file_write(filename, buff)
#endif


#define LOG_TIME_LEN 22
#define LOG_MAX_LEN  1024
#define MAX_LOG_FILES 64
#define MAX_FILE_NAME_SIZE 64
typedef struct log_manager {
    char pfiles[MAX_LOG_FILES][MAX_FILE_NAME_SIZE];
    int  size;
}log_manager;

static log_manager* g_log_manager = NULL;

static const char* level_str[] = {
    "DEBUG",
    "INFO",
    "NOTICE",
    "WARN",
    "ERROR",
    "CRIT",
    "ALERT",
    "EMERG"
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
int    flog_create(const char* file_name)
{
    if ( !g_log_manager ) {
        g_log_manager = malloc( sizeof( log_manager ) );
        if ( !g_log_manager ) return -1;

        g_log_manager->size = 0;
	int i = 0;
        for ( ; i < MAX_LOG_FILES; i++ ) {
            memset(g_log_manager->pfiles[i], 0, MAX_FILE_NAME_SIZE);
        }

        if ( FLOG_CREATE() ) return -2;
    }

    int log_handler = g_log_manager->size++;
    memcpy(g_log_manager->pfiles[log_handler], file_name, strlen(file_name));
    return log_handler;
}

inline
int    flog_set_level(int level)
{
    if( level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_EMERG )
        return 1; //error
    _f_log_level = level;
    return 0;
}

inline
int    flog_get_level()
{
    return _f_log_level;
}

int    flog(int log_handler, const char* src_filename, int lineno,
        const char* func_name, int level, const char* fmt, ...)
{
    if( level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_EMERG )
        return 1;
    char* filename = g_log_manager->pfiles[log_handler];
    if( !filename[0] ) return 2;
    if( !fmt ) return 3;

    char log_data[LOG_MAX_LEN];
    char now[LOG_TIME_LEN];
    va_list va;

    log_get_time(now);
    const char* _level_str = level_str[level];
    int head_len = sprintf(log_data, "%s [%s] %s:%d(%s) - ", now, _level_str,
            src_filename, lineno, func_name);

    va_start(va, fmt);
    vsnprintf(log_data + head_len, LOG_MAX_LEN - head_len - 2, fmt, va);
    va_end(va);

    FLOG(filename, log_data);

    return 0;
}

inline int is_debug_enable()  { return LOG_LEVEL_DEBUG  >= flog_get_level(); }
inline int is_info_enable()   { return LOG_LEVEL_INFO   >= flog_get_level(); }
inline int is_notice_enable() { return LOG_LEVEL_NOTICE >= flog_get_level(); }
inline int is_warn_enable()   { return LOG_LEVEL_WARN   >= flog_get_level(); }
inline int is_error_enable()  { return LOG_LEVEL_ERROR  >= flog_get_level(); }
inline int is_crit_enable()   { return LOG_LEVEL_CRIT   >= flog_get_level(); }
inline int is_alert_enable()  { return LOG_LEVEL_ALERT  >= flog_get_level(); }
inline int is_emerg_enable()  { return LOG_LEVEL_EMERG  >= flog_get_level(); }
