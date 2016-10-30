#ifndef FCONF_CORE_H
#define FCONF_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>

// note: the '#' in the content means note line

ssize_t fconf_load2buf(const char* filename, char* buf, size_t len);
int fconf_readline(const char* buf, int start, char* line);

int fconf_istoken (const char src, const char compare);
int fconf_istoken_blank(const char str);
int fconf_istoken_numsign(const char str);
int fconf_istoken_lf(const char str);
int fconf_istoken_null(const char str);

#ifdef __cplusplus
}
#endif

#endif
