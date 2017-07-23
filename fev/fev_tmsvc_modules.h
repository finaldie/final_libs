#ifndef _FEV_TIMER_SERVICE_MODULES_H_
#define _FEV_TIMER_SERVICE_MODULES_H_
/**
 *  This header file is the sdk header for module developers
 *  Note: This header dosen't expose to the users
 */

#include <time.h>

#include "flibs/fev.h"
#include "flibs/fev_tmsvc_types.h"

#define TIMEMS(tm) \
    (tm.tv_sec * MS_PER_SECOND + tm.tv_nsec / NS_PER_MS)

// The struct used for contain the basic information from user input
struct _ftimer_node {
    struct timespec start;
    ftimer_cb       cb;
    void*           arg;
    fev_timer_svc*  owner;
    long            expiration;
    int             isvalid;
    int             _padding;
};

// This operation table control the low level data storage/operation behaivor
// It's used for the library maintainers who can add/modify the new/exist modules
typedef struct fev_timer_svc_opt {
    int  (*init)   (void** mod_data);
    void (*destroy)(void*  mod_data);
    int  (*process)(fev_state*,   void* mod_data, struct timespec* now);
    int  (*add)    (ftimer_node*, void* mod_data);
    int  (*del)    (ftimer_node*, void* mod_data);
    ftimer_node* (*first)(void* mod_data);
} fev_tmsvc_opt;

/**
 * @desc If 'now' >= 'start + expire', means timeout
 *
 * @param start: the time of begin
 * @param now: the time of now
 * @param expire: the expiration time, unit is million second
 *
 * @return
 *  - 1: means already timeout
 *  - 0: means doesn't timeout
 */
int fev_tmmod_timeout(struct timespec* start,
                      struct timespec* now,
                      long expiration);

extern fev_tmsvc_opt* tmsvc_opt_tbl[];

#endif

