#ifndef _FEV_TIMER_SERVICE_MODULES_H_
#define _FEV_TIMER_SERVICE_MODULES_H_
/**
 *  This header file is the sdk header for module developers
 *  Note: This header dosen't expose to the users
 */

#include <stdint.h>
#include <time.h>

#include "flibs/fev.h"
#include "flibs/fev_tmsvc_types.h"

// The struct used for contain the basic information from user input
struct _ftimer_node {
    struct timespec start;
    ftimer_cb       cb;
    void*           arg;
    fev_timer_svc*  owner;
    uint32_t        expiration;
    int             isvalid;
};

// This operation table control the low level data storage/operation behaivor
// It's used for the library maintainers who can add/modify the new/exist modules
typedef struct fev_timer_svc_opt {
    int  (*init)(void** mod_data);
    void (*destroy)(void* mod_data);
    void (*loopcb)(fev_state*, void* mod_data, struct timespec* now);
    int  (*add)(ftimer_node*, void* mod_data);
    int  (*del)(ftimer_node*, void* mod_data);
    int  (*reset)(ftimer_node*, void* mod_data, uint32_t expiration);
} fev_tmsvc_opt;

// @desc: if 'now' >= 'start + expire', means timeout
// @param start: the time of begin
// @param now: the time of now
// @param expire: the expiration time, unit is million second
// @return 1, means already timeout
// @return 0, means doesn't timeout
int fev_tmmod_timeout(struct timespec* start,
                      struct timespec* now,
                      uint32_t expiration);

extern fev_tmsvc_opt* tmsvc_opt_tbl[];

#endif

