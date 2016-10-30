#include <stdlib.h>
#include "flibs/flog_level.h"

// define  global log level, set it to INFO level by default
static int flog_level = FLOG_LEVEL_INFO;

int flog_set_level(int level)
{
    if( level < FLOG_LEVEL_TRACE || level > FLOG_LEVEL_FATAL ) {
        return 1; //error
    }

    flog_level = level;
    return 0;
}

int    flog_get_level()
{
    return flog_level;
}

inline int flog_is_trace_enabled() {return FLOG_LEVEL_TRACE >= flog_get_level();}
inline int flog_is_debug_enabled() {return FLOG_LEVEL_DEBUG >= flog_get_level();}
inline int flog_is_info_enabled()  {return FLOG_LEVEL_INFO  >= flog_get_level();}
inline int flog_is_warn_enabled()  {return FLOG_LEVEL_WARN  >= flog_get_level();}
inline int flog_is_error_enabled() {return FLOG_LEVEL_ERROR >= flog_get_level();}
inline int flog_is_fatal_enabled() {return FLOG_LEVEL_FATAL >= flog_get_level();}
