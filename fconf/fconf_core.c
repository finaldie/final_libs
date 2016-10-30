#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fconf_core.h"

//return    0:normal 1:error
ssize_t     fconf_load2buf(const char* filename, char* buf, size_t len)
{
    int fd = 0;
    ssize_t bytes_read = 0;

    if ( (fd = open(filename, O_RDONLY)) == -1 ) {
        return -1;
    }

    bytes_read = read(fd, buf, len);
    if (bytes_read == -1)
        return -1;
    else if ( bytes_read > 0 ) {
        buf[bytes_read] = '\0';
    }

    return bytes_read;
}

int     fconf_istoken(const char src, const char cmp)
{
    if ( src == cmp )
        return 1;
    return 0;
}

int     fconf_istoken_null(const char str)
{
    return fconf_istoken(str, '\0');
}

int     fconf_istoken_lf(const char str)
{
    return fconf_istoken(str, '\n');
}

int     fconf_istoken_numsign(const char str)
{
    return fconf_istoken(str, '#');
}

int     fconf_istoken_blank(const char str)
{
    return fconf_istoken(str, ' ');
}

int     fconf_readline(const char* buf, int start, char* line)
{
    int len = 0;

    // skip all the blank, stop at the first non-blank location
    while ( !fconf_istoken_lf(buf[start]) ) {
        if ( fconf_istoken_blank(buf[start]) ) {
            start++;
        } else {
            break;
        }
    }

    if ( fconf_istoken_null(buf[start]) )
        return -1;

    // store the string before number sign('#')
    // people don't want to store the comments into
    // the buffer
    while ( !fconf_istoken_null(buf[start]) && !fconf_istoken_lf(buf[start]) ) {
        if ( !fconf_istoken_numsign(buf[start]) ) {
            line[len++] = buf[start++];
        } else {
            break;
        }
    }
    line[len] = '\0';

    // Move the 'start' pointer to the 1st char of the next line
    while ( !fconf_istoken_lf(buf[start]) )
        start++;

    return ++start;
}
