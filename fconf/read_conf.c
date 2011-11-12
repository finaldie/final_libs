#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "read_conf.h"

#define	CONF_BUFF_LEN 	1024 * 1024
#define	READ_LINE_LEN	128
#define	READ_WORD_LEN	32

int		IsTokenEqual( char pStr )
{
	return IsToken(pStr, '=');
}

/*
void	MatchRules(char* pWord, char* pValue)
{
	if( strcmp("ServerId", pWord) == 0 )
		cfgData->nServerId = atoi(pValue);
	else if( strcmp("ServerPort", pWord) == 0 )
		cfgData->nServerPort = atoi(pValue);
	else if( strcmp("ServerThreadNum", pWord) == 0 )
		cfgData->nServerThreadNum = atoi(pValue);
	else if( strcmp("DBPort", pWord) == 0 )
		cfgData->nDBPort = atoi(pValue);
	else if( strcmp("MaxConn", pWord) == 0 )
		cfgData->nMaxConn = atoi(pValue);
	else if( strcmp("ServerName", pWord) == 0 )
		strncpy( cfgData->strServerName, pValue, strlen(pValue)+1 );
	else if( strcmp("DBName", pWord) == 0 )
		strncpy( cfgData->strDBName, pValue, strlen(pValue)+1 );
	else if( strcmp("GateWayThreadNum", pWord) == 0 )
		cfgData->nGateWayThreadNum = atoi(pValue);
	else if( strcmp("ListenMax", pWord) == 0 )
		cfgData->nListenMax = atoi(pValue);
	else if( strcmp("ThreadPoolNum", pWord) == 0 )
		cfgData->nThreadPoolNum = atoi(pValue);
	else if( strcmp("ThreadRecvCache", pWord) == 0 )
		cfgData->nThreadRecvCache = atoi(pValue);
	else if( strcmp("GateWayPort", pWord) == 0 )
		cfgData->nGateWayPort = atoi(pValue);
	else if( strcmp("BalancePort", pWord) == 0 )
		cfgData->nBalancePort = atoi(pValue);
	else if( strcmp("GateWayIp", pWord) == 0 )
		strncpy(cfgData->strGateWayIp, pValue, strlen(pValue)+1);
	else
		printf("a nil value:%s\n", pWord);
}
*/

int		GenConfig(char* filename, pf_on_read pfunc)
{
	int		read_sign = 0;
	int		read_len  = 0;
	int		read_old_sign = 0;
	char	read_line[READ_LINE_LEN];
	char	read_word_left[READ_WORD_LEN];
	char	read_word_right[READ_WORD_LEN];

	char*	pConfBuf = (char*)malloc(CONF_BUFF_LEN);

	read_len = ReadConfig(filename, pConfBuf, CONF_BUFF_LEN);

	if( read_len <= 0 )
		return -1;
	
	read_sign = ReadLine(pConfBuf, read_old_sign, read_line);

	for(;read_sign > 0;)
	{
		int lp = 0;
		lp = ReadWord(read_line, read_word_left, lp);
		
		if( lp > 0 )
		{
			lp = ReadWord(read_line, read_word_right, lp);

			if( lp > 0 )
			{
				if ( IsTokenEqual(read_word_right[0]) )
				{
					lp = ReadWord(read_line, read_word_right, lp);
					
					if( lp > 0 )
						pfunc(read_word_left, read_word_right);
				}
			}
		}

		read_old_sign = read_sign;
		if( read_old_sign >= read_len )
			break;

		read_sign = ReadLine(pConfBuf, read_old_sign, read_line);
	}

	free(pConfBuf);
	
	return	0;
}

/*
int main(int argc, char** argv)
{
	cfg_Data* cfgData = &cfg;
	if( GenConfig("./server.conf", MatchRules) != 0 )
		printf("error gen\n");

	printf("serverid=%d\nserverport=%d\ndbport=%d\nservername=%s\ndbname=%s\nmaxconn=%d\n", cfgData->nServerId, 
		cfgData->nServerPort, cfgData->nDBPort, cfgData->strServerName, cfgData->strDBName, cfgData->nMaxConn);
	return 	0;
}
*/
