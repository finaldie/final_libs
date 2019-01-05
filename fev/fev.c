/*
 * =============================================================================
 *
 *       Filename:  fev.c
 *
 *    Description:  a light-weight event framework
 *
 *        Version:  1.0
 *        Created:  2011/11/13 15/59/50
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *        Company:
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "flibs/compiler.h"
#include "flibs/fev.h"
#include "flibs/fhash.h"

typedef struct fev_event {
    int         mask;       // READ OR WRITE
    int         fire_idx;   // we set the idx when the event has been disabled
                            // in one loop
    fev_read_cb  pread;
    fev_write_cb pwrite;
    void*        arg;
} fev_event;

struct fev_state {
    void*       state;
    fev_event*  fevents;
    int*        firelist;
    fhash*      module_tbl;
    int         max_ev_size;
    int         fire_num;
    int         in_processing;
    int         _reserved;       // unused
};

static void _fev_add_firelist(fev_state* fev, int fd)
{
    fev->firelist[ fev->fire_num ] = fd;
    fev->fevents[fd].fire_idx = fev->fire_num;
    fev->fire_num++;
}

static int _fev_is_fired(fev_state* fev, int fd)
{
    return fev->fevents[fd].fire_idx < fev->fire_num &&
        fev->firelist[fev->fevents[fd].fire_idx] == fd;
}

static void _fev_clear_firelist(fev_state* fev)
{
    fev->fire_num = 0;
}

#ifdef __linux__
#include "fev_epoll.c"
#else
#error "only support linux os now!"
#endif

#define FEV_DEFAULT_MODULE_CNT 10

fev_state*    fev_create(int max_ev_size)
{
    if (max_ev_size <= 0) {
        // get the current open file limitation (soft)
        struct rlimit limit;
        int ret = getrlimit(RLIMIT_NOFILE, &limit);
        if (ret) {
            perror("fev getrlimit failed");
            return NULL;
        }

        max_ev_size = (int)limit.rlim_cur;
    }

    fev_state* fev = (fev_state*)calloc(1, sizeof(fev_state));
    if (fev_state_create(fev, max_ev_size)) {
        perror("fev create state");
        free(fev);
        return NULL;
    }

    fev->fevents    = calloc(1, sizeof(fev_event) * (size_t)max_ev_size);
    fev->firelist   = calloc(1, sizeof(int) * (size_t)max_ev_size);
    fev->module_tbl = fhash_str_create(FEV_DEFAULT_MODULE_CNT,
                                       FHASH_MASK_AUTO_REHASH);

    fev->max_ev_size = max_ev_size;
    _fev_clear_firelist(fev);
    fev->in_processing = 0;

    for (int i = 0; i < max_ev_size; i++) {
        fev->fevents[i].arg      = NULL;
        fev->fevents[i].mask     = FEV_NIL;
        fev->fevents[i].pread    = NULL;
        fev->fevents[i].pwrite   = NULL;
        fev->fevents[i].fire_idx = 0;
    }

    return fev;
}

void    fev_destroy(fev_state* fev)
{
    if (!fev) { return; }

    // delete module's private data first
    fev_module_t* module = NULL;
    fhash_str_iter iter = fhash_str_iter_new(fev->module_tbl);
    while ((module = fhash_str_next(&iter))) {
        if (module->unload) {
            module->unload(fev, module->ud);
        }

        free(module);
    }
    fhash_str_iter_release(&iter);

    fhash_str_delete(fev->module_tbl);
    fev_state_destroy(fev);
    free(fev->fevents);
    free(fev->firelist);
    free(fev);
}

// return -1 : fev is null
// return -2 : mask flag must be one of FEV_READ or FEV_WRITE
// return -3 : fd already been registered
// return -4 : register event failed
// return  0 : sucess
int     fev_reg_event(fev_state* fev, int fd, int mask,
                      fev_read_cb pread, fev_write_cb pwrite, void* arg)
{
    if (unlikely(!fev)) { return -1; }
    if (unlikely(fev_get_mask(fev, fd) < 0)) { return -1; }

    // only reversed FEV_READ & FEV_WRITE state
    mask &= FEV_READ | FEV_WRITE;
    if (mask == FEV_NIL) { return -2; }

    if (unlikely(fev->fevents[fd].mask != FEV_NIL)) {
        return -3;
    }

    if (unlikely(fev_state_addevent(fev, fd, mask))) {
        return -4;
    }

    fev->fevents[fd].pread = pread;
    fev->fevents[fd].pwrite = pwrite;
    fev->fevents[fd].arg = arg;

    return 0;
}

int fev_add_event(fev_state* fev, int fd, int mask)
{
    if (unlikely(!fev)) { return -1; }
    if (unlikely(fev_get_mask(fev, fd) < 0)) { return -1; }

    // only reversed FEV_READ & FEV_WRITE state
    mask &= FEV_READ | FEV_WRITE;
    if (mask == FEV_NIL) { return 0; }

    if (unlikely(fev->fevents[fd].mask == FEV_NIL)) {
        return -2;
    }

    if (fev->fevents[fd].mask == mask) {
        return 0;
    }

    if (unlikely(fev_state_addevent(fev, fd, mask))) {
        return -3;
    }

    return 0;
}

// return -1 : fev is null
// return -2 : del event failed
// return  0 : sucess
int     fev_del_event(fev_state* fev, int fd, int mask)
{
    if (unlikely(!fev)) { return -1; }
    if (unlikely(fev_get_mask(fev, fd) < 0)) { return -1; }
    if (fev->fevents[fd].mask == FEV_NIL) { return 0; }

    // only reversed FEV_READ & FEV_WRITE state
    mask &= FEV_READ | FEV_WRITE;
    if (mask == FEV_NIL) { return 0; }

    if (fev_state_delevent(fev, fd, mask)) {
        return -2;
    }

    //finally if the fd's mask is FEV_NIL , then put the fd into firelist
    if (fev->fevents[fd].mask == FEV_NIL) {
        _fev_add_firelist(fev, fd);
    }

    return 0;
}

static
void _execute_module_prepoll(fev_state* fev) {
    fhash_str_iter iter = fhash_str_iter_new(fev->module_tbl);
    fev_module_t* module = NULL;

    while ((module = fhash_str_next(&iter))) {
        if (module->prepoll) {
            module->prepoll(fev, module->ud);
        }
    }
    fhash_str_iter_release(&iter);
}

static
void _execute_module_postpoll(fev_state* fev) {
    fhash_str_iter iter = fhash_str_iter_new(fev->module_tbl);
    fev_module_t* module = NULL;

    while ((module = fhash_str_next(&iter))) {
        if (module->postpoll) {
            module->postpoll(fev, module->ud);
        }
    }
    fhash_str_iter_release(&iter);
}

int     fev_poll(fev_state* fev, int timeout)
{
    if (unlikely(!fev)) { return 0; }

    if (unlikely(fev->in_processing)) {
        perror("fev_poll shouldn't support nest call");
        return -2;
    }

    fev->in_processing = 1;
    _fev_clear_firelist(fev);
    _execute_module_prepoll(fev);

    int num = fev_state_poll(fev, fev->max_ev_size, timeout);
    fev->in_processing = 0;

    _execute_module_postpoll(fev);
    return num;
}

int  fev_get_mask(fev_state* fev, int fd)
{
    if (unlikely(fd < 0 || fd >= fev->max_ev_size)) {
        return -1;
    }

    return fev->fevents[fd].mask;
}

int  fev_get_fd(fev_state* fev)
{
    if (unlikely(!fev)) { return -1; }
    return fev_state_getfd(fev);
}

int  fev_module_register(fev_state* fev, fev_module_t* module)
{
    if (fhash_str_get(fev->module_tbl, module->name)) {
        return 1;
    }

    fev_module_t* new_module = calloc(1, sizeof(fev_module_t));
    memcpy(new_module, module, sizeof(fev_module_t));

    fhash_str_set(fev->module_tbl, new_module->name, new_module);
    return 0;
}

void* fev_module_data(fev_state* fev, const char* module_name)
{
    fev_module_t* module = fhash_str_get(fev->module_tbl, module_name);
    if (!module) {
        return NULL;
    }

    return module->ud;
}
