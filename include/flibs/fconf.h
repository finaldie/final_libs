#ifndef FCONF_H
#define FCONF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*fconf_load_cfg_cb)(const char* key, const char* value);

// return 0: if loading config is ok
// return non-zero: if loading config is error
int fconf_load(const char* filename, fconf_load_cfg_cb pfunc);

#ifdef __cplusplus
}
#endif

#endif

