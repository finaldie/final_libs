//base info: create by hyz
/*effect: log system include file
*
*
*/

#ifndef _LOG_INCLUDE_H_
#define _LOG_INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_LEVEL_ERROR 0
#define	LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_ALERT 2
#define LOG_LEVEL_CRIT  3
#define LOG_LEVEL_WARN  4
#define LOG_LEVEL_EMERG 5
#define LOG_LEVEL_INFO  6
#define LOG_LEVEL_NOTICE 7

int	flog_create();
int	flog(int level, char* file_name, char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
