#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fconf_core.h"
#include "flibs/fconf.h"

#define CONF_BUFF_LEN (1024 * 1024)
#define READ_LINE_LEN 1024
#define READ_WORD_LEN 512

static
char* trim_left(char* str)
{
    if (!str) return NULL;
    if (*str == ' ') {
        return trim_left(str + 1);
    } else {
        return str;
    }

}

static
char* _trim_right(char* str, char* end) {
    if (*str == ' ') {
        *str = '\0';

        if (str == end) {
            return end;
        } else {
            return _trim_right(str - 1, end);
        }
    } else {
        return end;
    }
}

static
char* trim_right(char* str)
{
    if (!str) return NULL;

    size_t len = strlen(str);
    if (!len) return str;

    return _trim_right(str + len - 1, str);
}

char* trim(char* str)
{
    char* s1 = trim_left(str);
    return trim_right(s1);
}

int fconf_readkv(char* line, char** key, char** value)
{
    *key = NULL;
    *value = NULL;
    size_t lenofline = strlen(line);
    if ( lenofline == 0 ) {
        return -1;
    }

    // find the first '='
    char* loc = strchr(line, '=');
    if (!loc) {
        return -2;
    }

    // split it into key and value
    *key = line;
    *value = loc + 1;
    *loc = '\0'; // replace '=' with '\0'

    // trim the key and value
    *key = trim(*key);
    *value = trim(*value);

    // validate key, if key contain blanks, that's a
    // invalid key
    if (strchr(*key, ' ')) {
        return -3;
    }

    return 0;
}

int fconf_load(const char* filename, fconf_load_cfg_cb pfunc)
{
    int ret = 0;
    int next = 0;
    ssize_t buf_size = 0;
    char  line[READ_LINE_LEN];
    char* conf_buf = calloc(1, CONF_BUFF_LEN);

    buf_size = fconf_load2buf(filename, conf_buf, CONF_BUFF_LEN);

    // an empty config is a valid config
    if ( buf_size <= 0 )
        goto out;

    next = fconf_readline(conf_buf, next, line);

    while (next > 0) {
        if (*line == '\0') {
            goto next_round;
        }

        char* key = NULL;
        char* value = NULL;
        if (fconf_readkv(line, &key, &value)) {
            ret = 1;
            goto out;
        } else {
            pfunc(key, value);
        }

        // the next is a array index, the buf_size is a exact
        // size of the buffer, so compare them should convert
        // the next index to a normal size: next - 1 means how
        // many bytes we have already handled
        if ( (next - 1) >= buf_size ) {
            break;
        }

next_round:
        next = fconf_readline(conf_buf, next, line);
    }

out:
    free(conf_buf);
    return ret;
}
