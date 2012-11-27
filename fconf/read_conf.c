#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "read_conf.h"

#define CONF_BUFF_LEN (1024 * 1024)
#define READ_LINE_LEN 128
#define READ_WORD_LEN 32

int IsTokenEqual( char pStr )
{
    return IsToken(pStr, '=');
}

int GenConfig(const char* filename, pf_on_read pfunc)
{
    int   read_sign = 0;
    int   read_len  = 0;
    int   read_old_sign = 0;
    char  read_line[READ_LINE_LEN];
    char  read_word_left[READ_WORD_LEN];
    char  read_word_right[READ_WORD_LEN];
    char* pConfBuf = malloc(CONF_BUFF_LEN);

    read_len = ReadConfig(filename, pConfBuf, CONF_BUFF_LEN);

    if ( read_len <= 0 )
        return -1;

    read_sign = ReadLine(pConfBuf, read_old_sign, read_line);

    for ( ; read_sign > 0; ) {
        int lp = 0;
        lp = ReadWord(read_line, read_word_left, lp);

        if ( lp > 0 ) {
            lp = ReadWord(read_line, read_word_right, lp);

            if( lp > 0 ) {
                if ( IsTokenEqual(read_word_right[0]) ) {
                    lp = ReadWord(read_line, read_word_right, lp);

                    if ( lp > 0 )
                        pfunc(read_word_left, read_word_right);
                }
            }
        }

        read_old_sign = read_sign;
        if ( read_old_sign >= read_len )
            break;

        read_sign = ReadLine(pConfBuf, read_old_sign, read_line);
    }

    free(pConfBuf);

    return 0;
}
