#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "conf.h"

//return    0:normal 1:error
int     ReadConfig(const char* filename, char* pBuf, unsigned int len)
{
    int fd, bytes_read = 0;

    if ( (fd = open(filename, O_RDONLY)) == -1 ) {
        printf("open file error\n");
        return -1;
    }

    bytes_read = read(fd, pBuf, len);
    if ( (bytes_read == -1) )
        return    -1; 
    else if ( bytes_read > 0 ) {
        pBuf[bytes_read] = '\0';
    }

    return bytes_read;
}

int     IsToken(char pSrc, char pCmp)
{
    if ( pSrc == pCmp )
        return 1;
    return 0;
}

int     IsTokenEnd( char pStr )
{
    return IsToken(pStr, '\0');
}

int     IsTokenEnter( char pStr )
{
    return IsToken(pStr, '\n');
}

int     IsTokenNotes( char pStr )
{
    return IsToken(pStr, '#');
}

int     IsTokenBlank( char pStr )
{
    return IsToken(pStr, ' ');
}

int     ReadLine(char* pBufSrc, unsigned int Start, char* pLine)
{
    int len = 0;

    while ( !IsTokenEnd(pBufSrc[Start]) )
        if ( IsTokenBlank(pBufSrc[Start]) )
            Start++;
        else
            break;

    if ( IsTokenEnd(pBufSrc[Start]) )
        return -1;

    while ( !IsTokenEnd(pBufSrc[Start]) && !IsTokenEnter(pBufSrc[Start]) ) {
        if ( !IsTokenNotes(pBufSrc[Start]) ) {
            pLine[len++] = pBufSrc[Start++];
        } else {
            break;
        }
    }
    pLine[len] = '\0';

    while ( !IsTokenEnd(pBufSrc[Start]) && !IsTokenEnter(pBufSrc[Start]) )
        Start++;

    return ++Start;
}

int     ReadWord(char* pLine, char* pWord, int lp)
{
    int lp_word = 0;
    int lenofline = strlen(pLine);

    if ( lenofline == 0 )
        return -1;

    while ( !IsTokenEnd(pLine[lp]) )
        if ( IsTokenBlank(pLine[lp]) )
            lp++;
        else
            break;

    if ( IsTokenEnd(pLine[lp]) )
        return -1;

    while ( !IsTokenEnd(pLine[lp]) ) {
        if ( !IsTokenBlank(pLine[lp]) )
            pWord[lp_word++] = pLine[lp++];
        else
            break;
    }

    pWord[lp_word] = '\0';

    return lp;
}
