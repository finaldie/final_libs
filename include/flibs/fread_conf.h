//create by hyz
//desc : conf of enginex

#ifndef _FREAD_CONF_H_
#define _FREAD_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pfload_cfg_cb)(const char* key, const char* value);

// return 0: if loading config is ok
// return non-zero: if loading config is error
int fload_config(const char* filename, pfload_cfg_cb pfunc);

#ifdef __cplusplus
}
#endif

#endif
