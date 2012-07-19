//create by hyz
//desc : conf of enginex

#ifndef _READ_CONF_H_
#define _READ_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pf_on_read)(char* key, char* value);

int GenConfig(char* filename, pf_on_read pfunc);

#ifdef __cplusplus
}
#endif

#endif
