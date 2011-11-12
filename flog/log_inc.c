#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "log_inc.h"

#ifdef _USE_AIO_
#include "log_aio.h"

#define	 FLOG_CREATE()			log_aio_create()
#define  FLOG(filename, buff)	log_aio_write(filename, buff)

#else
#include "log.h"

#define  FLOG_CREATE()			log_create()
#define  FLOG(filename, buff) 	log_file_write(filename, buff)
#endif

#define LOG_TIME_LEN	22
#define LOG_MAX_LEN		1024	

static const char* level_str[] = {
	"error",
	"debug",
	"alert",
	"crit",
	"warn",
	"emerg",
	"info",
	"notice"
};

static
void	log_get_time(char* now_time){	// need buff len = 22
	time_t tm = time(NULL);
	struct tm* now = localtime(&tm);
	snprintf(now_time, LOG_TIME_LEN, "[%04d-%02d-%02d %02d:%02d:%02d]",
				(now->tm_year+1900), now->tm_mon+1, now->tm_mday,
				now->tm_hour, now->tm_min, now->tm_sec) ;
}

inline
int	flog_create(){
	return FLOG_CREATE();
}

int	flog(int level, char* file_name, char* fmt, ...){
	if( level < LOG_LEVEL_ERROR || level > LOG_LEVEL_NOTICE )
		return 1;
	if( !file_name ) return 2;
	if( !fmt ) return 3;

	char log_data[LOG_MAX_LEN];
	char now[LOG_TIME_LEN];
	int head_len = 0;
	va_list va;

	log_get_time(now);
	const char* _level_str = level_str[level];
	sprintf(log_data, "%s", now);
	sprintf(log_data + LOG_TIME_LEN - 1, "[%s]:", _level_str);
	head_len += LOG_TIME_LEN - 1;
	head_len += strlen(_level_str) + 3;	//fix the []:

	va_start(va, fmt);
	vsnprintf(log_data + head_len, LOG_MAX_LEN, fmt, va);
	va_end(va);

	FLOG(file_name, log_data);

	return 0;
}

