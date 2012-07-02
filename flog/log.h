//base info: create by hyz
/*effect: async log system
*
*
*/

#ifndef _LOG_SYSTEM_H_
#define _LOG_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

int log_create();
int    log_file_write(char* file_name, char* log);

#ifdef __cplusplus
}
#endif

#endif
