#ifndef FLOG_HELPERS_H
#define FLOG_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

// private, user no need to use the following 3 macros
#define FTO_STRX(x) #x
#define FTO_STR(x) FTO_STRX(x)

// public macors, flog advantage logger helper macros
// usage: FLOG_XXX(logger, "hello");
// usage: FLOG_XXX(logger, "%s", string);
#define FLOG_TRACE(log_handler, ...) \
    if( flog_is_trace_enable() ) { \
        flog_file_write_f(log_handler, \
                          __FILE__ ":" FTO_STR(__LINE__) " TRACE - " \
                          __VA_ARGS__); \
    }

#define FLOG_DEBUG(log_handler, ...) \
    if( flog_is_debug_enable() ) { \
        flog_file_write_f(log_handler, \
                          __FILE__ ":" FTO_STR(__LINE__) " DEBUG - " \
                          __VA_ARGS__); \
    }

#define FLOG_INFO(log_handler, ...) \
    if( flog_is_info_enable()  ) { \
        flog_file_write_f(log_handler, \
                          __FILE__ ":" FTO_STR(__LINE__) " INFO - " \
                          __VA_ARGS__); \
    }

#define FLOG_WARN(log_handler, ...) \
    if( flog_is_warn_enable()  ) { \
        flog_file_write_f(log_handler, \
                          __FILE__ ":" FTO_STR(__LINE__) " WARN - " \
                          __VA_ARGS__); \
    }

#define FLOG_ERROR(log_handler, ...) \
    if( flog_is_error_enable() ) { \
        flog_file_write_f(log_handler, \
                          __FILE__ ":" FTO_STR(__LINE__) " ERROR - " \
                          __VA_ARGS__); \
    }

#define FLOG_FATAL(log_handler, ...) \
    if( flog_is_fatal_enable() ) { \
        flog_file_write_f(log_handler, \
                          __FILE__ ":" FTO_STR(__LINE__) " FATAL - " \
                          __VA_ARGS__); \
    }

#ifdef __cplusplus
}
#endif

#endif
