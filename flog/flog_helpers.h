#ifndef FLOG_HELPERS_H
#define FLOG_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

// private, user no need to use the following 3 macros
#define FTO_STRX(x) #x
#define FTO_STR(x) FTO_STRX(x)

// flog basic logger helper macros
#define FLOG_RAW_TRACE(log_handler, ...) \
    if( flog_is_trace_enable() ) { \
        flog_file_write_f(log_handler, __VA_ARGS__); \
    }

#define FLOG_RAW_DEBUG(log_handler, ...) \
    if( flog_is_debug_enable() ) { \
        flog_file_write_f(log_handler, __VA_ARGS__); \
    }

#define FLOG_RAW_INFO(log_handler, ...) \
    if( flog_is_info_enable() ) { \
        flog_file_write_f(log_handler, __VA_ARGS__); \
    }

#define FLOG_RAW_WARN(log_handler, ...) \
    if( flog_is_warn_enable() ) { \
        flog_file_write_f(log_handler, __VA_ARGS__); \
    }

#define FLOG_RAW_ERROR(log_handler, ...) \
    if( flog_is_error_enable() ) { \
        flog_file_write_f(log_handler, __VA_ARGS__); \
    }

#define FLOG_RAW_FATAL(log_handler, ...) \
    if( flog_is_fatal_enable() ) { \
        flog_file_write_f(log_handler, __VA_ARGS__); \
    }

// flog advantage logger helper macros
#define FLOG_TRACE(log_handler, ...) \
    if( flog_is_trace_enable() ) { \
        flog_file_write_f(log_handler, \
                          "[TRACE] " __FILE__ ":" FTO_STR(__LINE__) " - " \
                          __VA_ARGS__); \
    }

#define FLOG_DEBUG(log_handler, ...) \
    if( flog_is_debug_enable() ) { \
        flog_file_write_f(log_handler, \
                          "[DEBUG] " __FILE__ ":" FTO_STR(__LINE__) " - " \
                          __VA_ARGS__); \
    }

#define FLOG_INFO(log_handler, ...) \
    if( flog_is_info_enable()  ) { \
        flog_file_write_f(log_handler, \
                          "[INFO] "  __FILE__ ":" FTO_STR(__LINE__) " - " \
                          __VA_ARGS__); \
    }

#define FLOG_WARN(log_handler, ...) \
    if( flog_is_warn_enable()  ) { \
        flog_file_write_f(log_handler, \
                          "[WARN] "  __FILE__ ":" FTO_STR(__LINE__) " - " \
                          __VA_ARGS__); \
    }

#define FLOG_ERROR(log_handler, ...) \
    if( flog_is_error_enable() ) { \
        flog_file_write_f(log_handler, \
                          "[ERROR] " __FILE__ ":" FTO_STR(__LINE__) " - " \
                          __VA_ARGS__); \
    }

#define FLOG_FATAL(log_handler, ...) \
    if( flog_is_fatal_enable() ) { \
        flog_file_write_f(log_handler, \
                          "[FATAL] " __FILE__ ":" FTO_STR(__LINE__) " - " \
                          __VA_ARGS__); \
    }

#ifdef __cplusplus
}
#endif

#endif
