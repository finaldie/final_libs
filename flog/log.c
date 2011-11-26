//base info: create by hyz
//effect: async log system

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "lmutex.h"
#include "mbuf.h"
#include "lhash.h"
#include "log.h"

#define LOG_MAX_BUFF_SIZE	1024 * 512
#define LOG_OPEN_PERMISSION 0755

//TODO...

typedef struct{
	int 	len;
	char* 	data;
}data_info;

typedef struct{
	mbuf*		pbuf;
}log_file;

typedef struct _log_t{
	mbuf* 		pqueue;
	f_hash*		phash;
	spin_var	log_lock;
	cond_var 	log_cond;
}f_log;

// define global log struct
static f_log*	g_log;

// global buff
static char* 	g_buff;

static inline
int		log_write(int fd, char* log, int len){
	return write(fd, log, len);
}

static inline
int		log_open(char* file_name){
	return open(file_name, O_CREAT | O_WRONLY | O_APPEND, LOG_OPEN_PERMISSION);
}

static
log_file*	log_create_file(){
	log_file* pfile = (log_file*)malloc(sizeof(log_file));
	pfile->pbuf = mbuf_create(LOG_MAX_BUFF_SIZE);

	return pfile;
}

static
int		log_push(char* file_name, char* log){
	spin_lock(&g_log->log_lock);

	log_file* lfile = (log_file*)hash_get_str(g_log->phash, file_name);

	if( !lfile ){
		lfile = log_create_file();
		hash_set_str(g_log->phash, file_name, lfile);
	}

	int flen = strlen(file_name) + 1;
	mbuf_push(g_log->pqueue, &flen, sizeof(flen));
	mbuf_push(g_log->pqueue, file_name, flen);

	int dlen = strlen(log) + 1;
	mbuf_push(lfile->pbuf, &dlen, sizeof(dlen));
	mbuf_push(lfile->pbuf, log, dlen);

	spin_unlock(&g_log->log_lock);

	return 0;
}

static
char*	log_pop_filename(char* buff){
	char* file_name = NULL;
	spin_lock(&g_log->log_lock);

	int flen = 0;
	mbuf_pop(g_log->pqueue, &flen, sizeof(flen));
	if( flen ){
		mbuf_pop(g_log->pqueue, buff, flen);
		file_name = buff;
	}

	spin_unlock(&g_log->log_lock);

	return file_name;
}

static
int		log_pop_data(char* file_name, char* buff, data_info* dinfo){
	log_file* lfile = (log_file*)hash_get_str(g_log->phash, file_name);
	if( !lfile ) return 1;

	int dlen;
	int* len = (int*)mbuf_getraw(lfile->pbuf, &dlen, sizeof(dlen));
	if( !len ) return 2;
	
	char* data = (char*)mbuf_vpop(lfile->pbuf, buff, *len + sizeof(int));
	if( !data ) return 3;

	dinfo->len = *len - 1;
	dinfo->data = data + sizeof(int);

	return 0;
}

void*	log_work(void* arg){
	printf("log work thread start\n");
	char filename_buf[40];

LOG_LOOP:
	cond_wait(&g_log->log_cond);

	for(;;){
		char* file_name = log_pop_filename(filename_buf);
		if( !file_name ) break;

		int fd = log_open(file_name);
		if( fd == -1 ){
			printf("fileopen error=%d(%s)\n", errno, strerror(errno));
			exit(1);
		}

		data_info dinfo;
		if( !log_pop_data(file_name, g_buff, &dinfo) ){
			int len = dinfo.len;
			char* data = dinfo.data;

			log_write(fd, data, len);
		}

		close(fd);
	}	
	
	goto LOG_LOOP;

	return NULL;
}

static
void	create_log_thread(){
	pthread_t tid;
	int rc = pthread_create(&tid, 0, log_work, NULL);

	if( rc != 0 )
	{
		printf("create send thread failed\n");
		exit(1);
	}
}

int		log_create(){
	if( !g_log ){
		g_log = (f_log*)malloc(sizeof(f_log));	
		g_log->phash = hash_create(0);
		g_log->pqueue = mbuf_create(LOG_MAX_BUFF_SIZE);
		spin_init(&g_log->log_lock);
		cond_init(&g_log->log_cond);
		g_buff = (char*)malloc(LOG_MAX_BUFF_SIZE);

		create_log_thread();
	}

	return 0;
}

int		log_file_write(char* file_name, char* log){
	if( !g_log && log_create() ){}

	log_push(file_name, log);
	cond_wakeup(&g_log->log_cond);

	return 0;
}

/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/
