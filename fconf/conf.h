#ifndef _CONF_H_FINAL_
#define _CONF_H_FINAL_

#ifdef __cplusplus
extern "C" {
#endif

// note: the '#' in the content means note line

int ReadConfig(char* filename, char* pBuf, unsigned int len);
int ReadLine(char* pBufSrc, unsigned int Start, char* pLine);
int ReadWord(char* pLine, char* pWord, int lp);
int IsToken(char pSrc, char pCmp);

#ifdef __cplusplus
}
#endif

#endif
