//base info: create by hyz
/*effect: log with laio_write
*
*
*/


#ifndef _LOG_WITH_AIO_H_
#define	_LOG_WITH_AIO_H_

#ifdef __cplusplus
extern "C" {
#endif

int	log_aio_create();
int	log_aio_write(char* file_name, char* buff);

#ifdef __cplusplus
}
#endif

#endif
