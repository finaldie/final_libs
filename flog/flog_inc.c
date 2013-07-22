#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "flog.h"
#include "flog_inc.h"

static int _f_log_level = LOG_LEVEL_INFO;

inline
log_file_t* flog_create(const char* file_name)
{
    return log_create(file_name);
}

void flog_destroy(log_file_t* fl)
{
    log_destroy(fl);
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

void flog_set_mode(LOG_MODE mode)
{
    log_set_mode(mode);
}

void flog_set_roll_size(size_t size)
{
    log_set_roll_size(size);
}

void flog_set_flush_interval(size_t sec)
{
    log_set_flush_interval(sec);
}

void flog_set_buffer_size(size_t size)
{
    log_set_buffer_size(size);
}

size_t flog_get_buffer_size()
{
    return log_get_buffer_size();
}

void flog_register_event_callback(plog_event_func pfunc)
{
    log_register_event_callback(pfunc);
}

inline int is_trace_enable()  { return LOG_LEVEL_TRACE  >= flog_get_level(); }
inline int is_debug_enable()  { return LOG_LEVEL_DEBUG  >= flog_get_level(); }
inline int is_info_enable()   { return LOG_LEVEL_INFO   >= flog_get_level(); }
inline int is_warn_enable()   { return LOG_LEVEL_WARN   >= flog_get_level(); }
inline int is_error_enable()  { return LOG_LEVEL_ERROR  >= flog_get_level(); }
inline int is_fatal_enable()  { return LOG_LEVEL_FATAL  >= flog_get_level(); }
