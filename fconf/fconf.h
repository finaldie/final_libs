#ifndef _CONF_H_FINAL_
#define _CONF_H_FINAL_

#ifdef __cplusplus
extern "C" {
#endif

// note: the '#' in the content means note line

int fconf_load2buf(const char* filename, char* buf, size_t len);
int fconf_readline(const char* buf, unsigned int start, char* line);

int fconf_istoken (const char src, const char compare);
int fconf_istoken_blank(const char str);
int fconf_istoken_numsign(const char str);
int fconf_istoken_lf(const char str);
int fconf_istoken_null(const char str);

#ifdef __cplusplus
}
#endif

#endif
