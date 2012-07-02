//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "core.h"
#include "log_aio.h"
#include "laio_write.h"
#include "lhash.h"
#include "lmutex.h"

#define LOG_OPEN_PERMISSION 0755

//TODO...

typedef struct{
    f_hash*     phash;
    spin_var lock;
}log_aio;

typedef struct{
    int fd;
}log_idata;

typedef struct{
    laio_w        aio_t;
    int         len;
    char        log_data[0];
}log_file;

log_aio g_log;

static inline
int        lopen(char* file_name){
    return open(file_name, O_CREAT | O_WRONLY | O_APPEND, LOG_OPEN_PERMISSION);
}

int        log_complete(void* data, int len){
    free(data);
    return 0;
}

int        log_aio_create(){
    g_log.phash = hash_create(0);
    spin_init(&g_log.lock);

    return 0;
}

int        log_aio_getfile(char* file_name){
    int* _fd = (int*)hash_get_str(g_log.phash, file_name);

    if( !_fd ){
        spin_lock(&g_log.lock);

        log_idata* ld = (log_idata*)malloc(sizeof(log_idata));
        int fd = lopen(file_name);
        ld->fd = fd;
        hash_set_str(g_log.phash, file_name, ld);

        spin_unlock(&g_log.lock);

        return ld->fd;
    }

    return *_fd;
}

int        log_aio_write(char* file_name, char* buff){
    int fd = log_aio_getfile(file_name);
    int len = strlen(buff);
    log_file* lfile = (log_file*)malloc(sizeof(log_file) + len);

    lfile->len = len;
    strncpy(lfile->log_data, buff, len);
    laio_init(&lfile->aio_t, log_complete, lfile);

    laio_write(&lfile->aio_t, fd, lfile->log_data, len);

    return 0;
}

/*
int    main(int argc, char** argv)
{
    return 0;
}

*/
