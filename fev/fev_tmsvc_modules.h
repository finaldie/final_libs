#ifndef _FEV_TIMER_SERVICE_MODULES_H_
#define _FEV_TIMER_SERVICE_MODULES_H_
/**
 *  This header file is the sdk header for module developers
 *  Note: This header dosen't expose to the users
 */

#ifndef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <time.h>
#include <stdint.h>

#include "flibs/fev.h"
#include "flibs/fev_tmsvc_types.h"

#define NS_PER_SECOND 1000000000L
#define NS_PER_MS     1000000L
#define MS_PER_SECOND 1000

#define TIMEMS(tm) \
    ((tm).tv_sec * MS_PER_SECOND + (tm).tv_nsec / NS_PER_MS)

// The struct used for contain the basic information from user input
struct _ftimer_node {
    struct timespec start;
    ftimer_cb       cb;
    void*           arg;
    fev_timer_svc*  owner;

    long            interval;
    long            expiration;

    uint32_t        valid     :1;
    uint32_t        triggered :1;

    uint32_t        _padding  :30;
    int             _padding1;
};

// This operation table control the low level data storage/operation behaivor
// It's used for the library maintainers who can add/modify the new/exist modules
typedef struct fev_timer_svc_ops_t {
    int  (*init)   (void** mod_data);
    void (*destroy)(void*  mod_data);
    int  (*process)(fev_state*,   void* mod_data, struct timespec* now);
    int  (*add)    (ftimer_node*, void* mod_data);
    int  (*del)    (ftimer_node*, void* mod_data);
    ftimer_node* (*top)(void* mod_data);
} fev_tmsvc_ops_t;

/**
 * @desc If 'now' >= 'start + expire', means timeout
 *
 * @return
 *  - 1: means already timeout
 *  - 0: means doesn't timeout
 */
int fev_tmmod_timeout(ftimer_node* node, struct timespec* now);

extern fev_tmsvc_ops_t* tmsvc_ops_tbl[];

#endif

