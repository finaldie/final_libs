#ifndef FLOG_HELPERS_H
#define FLOG_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdarg.h>

// private, user no need to use the following 3 macros
#define FTO_STRX(x) #x
#define FTO_STR(x) FTO_STRX(x)
#define FEXTRACT_STR(s) s, (sizeof(s) - 1)

// flog basic logger helper
#define FLOG_TRACE(log_handler, fmt, args...) \
    if( flog_is_trace_enable() ) { \
        flog_file_write_f(log_handler, FEXTRACT_STR("[TRACE] " __FILE__ ":" \
                                                   FTO_STR(__LINE__) " - "), \
                          fmt, ##args); \
    }

#define FLOG_DEBUG(log_handler, fmt, args...) \
    if( flog_is_debug_enable() ) { \
        flog_file_write_f(log_handler, FEXTRACT_STR("[DEBUG] " __FILE__ ":" \
                                                   FTO_STR(__LINE__) " - "), \
                          fmt, ##args); \
    }

#define FLOG_INFO(log_handler,  fmt, args...) \
    if( flog_is_info_enable()  ) { \
        flog_file_write_f(log_handler, FEXTRACT_STR("[INFO] "  __FILE__ ":" \
                                                   FTO_STR(__LINE__) " - "), \
                          fmt, ##args); \
    }

#define FLOG_WARN(log_handler,  fmt, args...) \
    if( flog_is_warn_enable()  ) { \
        flog_file_write_f(log_handler, FEXTRACT_STR("[WARN] "  __FILE__ ":" \
                                                   FTO_STR(__LINE__) " - "), \
                          fmt, ##args); \
    }

#define FLOG_ERROR(log_handler, fmt, args...) \
    if( flog_is_error_enable() ) { \
        flog_file_write_f(log_handler, FEXTRACT_STR("[ERROR] " __FILE__ ":" \
                                                   FTO_STR(__LINE__) " - "), \
                          fmt, ##args); \
    }

#define FLOG_FATAL(log_handler, fmt, args...) \
    if( flog_is_fatal_enable() ) { \
        flog_file_write_f(log_handler, FEXTRACT_STR("[FATAL] " __FILE__ ":" \
                                                   FTO_STR(__LINE__) " - "), \
                          fmt, ##args); \
    }

#ifdef __cplusplus
}
#endif

#endif
