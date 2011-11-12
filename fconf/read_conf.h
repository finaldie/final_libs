//create by hyz
//desc : conf of enginex

#ifndef _READ_CONF_H_
#define _READ_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef	struct
{
	unsigned int	nServerId;
	unsigned int	nServerPort;
	unsigned int 	nServerThreadNum;
	unsigned int 	nDBPort;
	unsigned int	nMaxConn;
	unsigned int	nThreadPoolNum;
	unsigned int 	nGateWayThreadNum;
	unsigned int	nGateWayPort;
	unsigned int	nListenMax;
	unsigned int 	nThreadRecvCache;
	unsigned int	nBalancePort;
	char			strServerName[20];
	char			strDBName[20];
	char			strGateWayIp[16];

}cfg_Data;
*/

typedef void (*pf_on_read)(char* key, char* value);

int		GenConfig(char* filename, pf_on_read pfunc);

#ifdef __cplusplus
}
#endif

#endif
