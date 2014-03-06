/**
 *  This header file is the sdk header for module developers
 *  Note: This header dosen't expose to the users
 */

#ifndef _FEV_TIMER_SERVICE_MODULES_H_
#define _FEV_TIMER_SERVICE_MODULES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>
#include "fev.h"
#include "fev_tmsvc_types.h"

// The struct used for contain the basic information from user input
struct _ftimer_node {
    struct timespec start;
    ftimer_cb       cb;
    void*           arg;
    fev_timer_svc*  owner;
    uint32_t        expire;
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
    int  (*reset)(ftimer_node*, void* mod_data);
} fev_tmsvc_opt;

// if 'now' >= 'start + expire', means timeout
// @return 1, means already timeout
// @return 0, means doesn't timeout
int fev_tmmod_timeout(struct timespec* start,
                        struct timespec* now,
                        uint32_t expire);

extern fev_tmsvc_opt* tmsvc_opt_tbl[];

#ifdef __cplusplus
}
#endif

#endif
