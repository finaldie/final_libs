#ifndef FEV_H
#define FEV_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fev_state fev_state;

// fev fd mask
#define FEV_NIL     0x0
#define FEV_READ    0x1
#define FEV_WRITE   0x2
#define FEV_ERROR   0x4

typedef void (*fev_read_cb)(fev_state*, int fd, int mask, void* arg);
typedef void (*fev_write_cb)(fev_state*, int fd, int mask, void* arg);

fev_state* fev_create(int max_ev_size);
void fev_destroy(fev_state*);
int  fev_poll(fev_state*, int timeout);

int  fev_reg_event(fev_state*, int fd, int mask, fev_read_cb, fev_write_cb, void* arg);
int  fev_add_event(fev_state*, int fd, int mask);
int  fev_del_event(fev_state*, int fd, int mask);
int  fev_get_mask(fev_state*, int fd);
int  fev_get_fd(fev_state*);

typedef struct fev_module_t {
    const char* name;
    void* ud;
    void  (*fev_module_unload)(fev_state*, void* ud);
} fev_module_t;

int   fev_register_module(fev_state*, fev_module_t*);
void* fev_get_module_data(fev_state*, const char* module_name);

#ifdef __cplusplus
}
#endif

#endif

