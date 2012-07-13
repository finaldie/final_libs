#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

#include "mbuf.h"
#include "lhash.h"
#include "log.h"

#define LOG_OPEN_PERMISSION        0755
#define LOG_MAX_THREAD_BUFF_SIZE   (1024 * 1024)
#define LOG_MAX_FILE_NAME          (64)
#define LOG_MAX_OUTPUT_NAME        (128)
#define LOG_BUFFER_SIZE_PER_FILE   (1024 * 64)
#define LOG_DEFAULT_ROLL_SIZE      (1024 * 1024 * 1024)
#define LOG_DEFAULT_FLUSH_INTERVAL (0)
#define LOG_MAX_LEN_PER_MSG        (4096)
#define LOG_MSG_HEAD_SIZE          ( sizeof(log_msg_head_t) )

// every file have only one log_file structure data
struct _log_file_t {
    FILE*  pf;
    size_t file_size;
    time_t last_flush_time;
    char   pfilename[LOG_MAX_FILE_NAME];
    char   poutput_filename[LOG_MAX_OUTPUT_NAME];
    char   filebuf[LOG_BUFFER_SIZE_PER_FILE];
    size_t ref_count;
};

// log message header
#pragma pack(2)
typedef struct {
    log_file_t*    f;
    unsigned short len;
}log_msg_head_t;
#pragma pack()

// every thread have a private thread_data for containing logbuf
// when we write log messages, data will fill into this buffer
typedef struct _thread_data_t {
    mbuf*        plog_buf;
    int          efd;      // eventfd
    unsigned int write_count;
    unsigned int read_count;
}thread_data_t;

// global log system data
typedef struct _log_t {
    f_hash*         phash;  // mapping filename <--> log_file structure
    pthread_mutex_t lock;   // protect some scences resource competion
    pthread_key_t   key;
    LOG_MODE        mode;   // global log flag
    int             is_background_thread_started;
    int             roll_size;
    int             flush_interval;
    int             epfd;   // epoll fd
}f_log;

// define global log struct
static f_log* g_log = NULL;

// ensure g_log only initialization once
static pthread_once_t init_create = PTHREAD_ONCE_INIT;

#define printError(msg) \
    fprintf(stderr, "FATAL! Log system:[%s:%s(%d)] - errno=%d errmsg=%s:%s\n",\
            __FILE__, __func__, __LINE__, errno, strerror(errno), msg);

/******************************************************************************
 * Internal functions
 *****************************************************************************/
static inline
int _lopen(const char* filename){
    return open(filename, O_CREAT | O_WRONLY | O_APPEND, LOG_OPEN_PERMISSION);
}

static inline
FILE* _log_open(const char* filename, char* buf, int size){
    int fd = _lopen(filename);
    if ( fd < 0 ) {
        printError("open file failed\n");
        return NULL;
    }

    FILE* f = fdopen(fd, "a");
    if ( NULL == f ) {
        printError("open file failed\n");
        return NULL;
    }

    setbuffer(f, buf, size);
    return f;
}

static inline
char* _log_generate_filename(const char* filename, char* output_filename)
{
    char now_time[22];
    time_t tm_time = time(NULL);
    struct tm now;
    gmtime_r(&tm_time, &now);
    snprintf(now_time, 22, "%04d_%02d_%02d_%02d_%02d_%02d",
                (now.tm_year+1900), now.tm_mon+1, now.tm_mday,
                now.tm_hour, now.tm_min, now.tm_sec);

    snprintf(output_filename, LOG_MAX_OUTPUT_NAME, "%s_%s.%d",
            filename, now_time, getpid());
    return output_filename;
}

static inline
void _log_flush_file(log_file_t* lf, time_t now)
{
    if ( fflush(lf->pf) ) {
        printError("cannot flush file");
        return;
    }

    lf->last_flush_time = now;
}

static inline
void _log_roll_file(log_file_t* lf)
{
    _log_generate_filename(lf->pfilename, lf->poutput_filename);
    FILE* pf = _log_open(lf->poutput_filename, lf->filebuf,
                         LOG_BUFFER_SIZE_PER_FILE);
    if ( !pf ) {
        printError("open file failed");
        return;
    }

    fclose(lf->pf);
    lf->pf = pf;
    lf->file_size = 0;
}

static inline
size_t _log_write_unlocked(log_file_t* lf, const char* log, size_t len)
{
    if ( !lf || !log || (len == 0) ) {
        printError("cannot log any msg");
        return 0;
    }

    FILE* f = lf->pf;
    size_t remain_len = len;

    do {
        size_t real_len = fwrite_unlocked(log, 1, len, f);
        if ( real_len > 0 ) {
            remain_len -= real_len;
        } else {
            // encounter error
            int err = ferror(lf->pf);
            if ( err ) {
                printError("encounter error when writing logging message");
                return len - remain_len;
            }
        }
    } while( remain_len );

    return len - remain_len;
}

/**
 * Asynchronous mode we use a large ring buffer to hold the log messages data,
 * the format of log message is:
 * log_file_ptr(4/8 bytes) | len(2 bytes) | log message(len bytes)
 */
static
size_t _log_async_write(log_file_t* f, const char* log, size_t len)
{
    if ( !f || !log || len == 0 ) {
        return 0;
    }

    // check whether have private thread data, if not, create it
    thread_data_t* th_data = pthread_getspecific(g_log->key);
    if ( !th_data ) {
        th_data = malloc( sizeof (thread_data_t) );
        if ( !th_data ) {
            printError("cannot alloc memory for thread data");
            return 0;
        }

        th_data->write_count = 0;
        th_data->read_count = 0;

        mbuf* pbuf = mbuf_create(LOG_MAX_THREAD_BUFF_SIZE);
        if ( !pbuf ) {
            printError("cannot alloc memory for thread data");
            free(th_data);
            return 0;
        }
        th_data->plog_buf = pbuf;

        int efd = eventfd(0, EFD_NONBLOCK);
        if ( efd == -1 ) {
            printError("cannot create eventfd for thread data");
            free(th_data);
            return 0;
        }
        th_data->efd = efd;

        struct epoll_event ee;
        ee.data.u64 = 0;
        ee.data.fd = 0;
        ee.data.ptr = th_data;
        ee.events = 0;
        ee.events |= EPOLLIN;
        if ( epoll_ctl( g_log->epfd, EPOLL_CTL_ADD, efd, &ee) ) {
            printError("cannot add eventfd into epoll for thread data");
            free(th_data);
            return 0;
        }

        pthread_setspecific(g_log->key, th_data);
    }

    // wrap and push log message
    if ( len > LOG_MAX_LEN_PER_MSG) {
        return 0;
    }

    size_t total_msg_len = LOG_MSG_HEAD_SIZE + len;
    if ( mbuf_free(th_data->plog_buf) < (int)total_msg_len ) {
        return 0;
    }

    log_msg_head_t header;
    header.f = f;
    header.len = (unsigned short)len;
    if ( mbuf_push(th_data->plog_buf, &header, LOG_MSG_HEAD_SIZE) ) {
        return 0;
    }

    if( mbuf_push(th_data->plog_buf, log, len) ) {
        return 0;
    }

    th_data->write_count++;

    // notice fetcher to write log
    if ( eventfd_write(th_data->efd, 1) ) {
        return 0;
    }

    return len;
}

static
size_t _log_sync_write(log_file_t* f, const char* log, size_t len)
{
    pthread_mutex_lock(&g_log->lock);
    size_t real_writen_len = _log_write_unlocked(f, log, len);
    if ( real_writen_len < len ) {
        // error
    }

    f->file_size += real_writen_len;
    time_t now = time(NULL);
    if ( now - f->last_flush_time >= g_log->flush_interval ) {
        _log_flush_file(f, now);
    }

    if ( f->file_size > g_log->roll_size ) {
        _log_roll_file(f);
    }

    pthread_mutex_unlock(&g_log->lock);

    return real_writen_len;
}

static inline
void _log_async_process(thread_data_t* th_data, unsigned int process_num)
{
    unsigned int i;
    mbuf* pbuf = th_data->plog_buf;
    char tmp_buf[LOG_MAX_LEN_PER_MSG];
    char* tmsg = NULL;
    log_msg_head_t header;

    for (i=0; i<process_num; i++) {
        if ( mbuf_pop(pbuf, &header, LOG_MSG_HEAD_SIZE) ) {
            break;
        }

        tmsg = mbuf_getraw(pbuf, tmp_buf, (size_t)header.len);
        if ( !tmsg ) {
            break;
        }

        size_t real_writen_len = _log_write_unlocked(header.f, tmsg,
                                                    (size_t)header.len);
        if ( real_writen_len < (size_t)header.len ) {
            printf("cannot write whole log message\n");
        }
        mbuf_head_move(pbuf, (size_t)header.len);

        header.f->file_size += real_writen_len;
        time_t now = time(NULL);
        if ( now - header.f->last_flush_time >= g_log->flush_interval ) {
            _log_flush_file(header.f, now);
        }

        if ( header.f->file_size > g_log->roll_size ) {
            _log_roll_file(header.f);
        }
    }
}

static inline
int _log_process_timeout(void* arg)
{
    log_file_t* f = (log_file_t*)arg;
    time_t now = time(NULL);
    if ( now - f->last_flush_time >= g_log->flush_interval ) {
        _log_flush_file(f, now);
    }

    return 0;
}

static
void* _log_fetcher(void* arg){
    printf("log work thread start\n");
    int nums, i;
    struct epoll_event events[1024];
    uint64_t notice;
    pthread_detach(pthread_self());

LOG_LOOP:
    nums = epoll_wait(g_log->epfd, events, 1024, 1000);
    if ( nums < 0 && errno == EINTR )
        goto LOG_LOOP;

    // timeout
    if ( nums == 0 ) {
        hash_foreach(g_log->phash, _log_process_timeout);
    }

    for(i=0; i<nums; i++) {
        struct epoll_event* ee = &events[i];
        thread_data_t* th_data = ee->data.ptr;
        eventfd_read(th_data->efd, &notice);

        unsigned int write_count = th_data->write_count;
        unsigned int read_count = th_data->read_count;

        unsigned int process_num = 0;
        if ( read_count != write_count ) {
            process_num = write_count > read_count ?
                            write_count - read_count :
                            0xFFFFFFFF - ( write_count - read_count) + 1;
            _log_async_process(th_data, process_num);
        }
    }

    goto LOG_LOOP;

    return NULL;
}

static
void _create_fetcher_thread(){
    pthread_t tid;
    int rc = pthread_create(&tid, NULL, _log_fetcher, NULL);
    if( rc )
    {
        printError("create send thread failed\n");
        exit(1);
    }
}

static
void _user_thread_destroy(void* arg)
{
    thread_data_t* th_data = (thread_data_t*)arg;

    if ( !th_data ) return;
    if ( th_data->plog_buf )
        mbuf_delete(th_data->plog_buf);
    close(th_data->efd);
    free(th_data);
}

static
void _log_init()
{
    f_log* t_log = malloc(sizeof(f_log));
    if ( !t_log ) {
        printError("cannot alloc memory for global log data");
        exit(1);
    }

    int epfd = epoll_create(1024);
    if ( epfd == -1 ) {
        printError("cannot create epollfd for global log data");
        exit(1);
    }

    t_log->epfd = epfd;
    t_log->phash = hash_create(0);
    pthread_mutex_init(&t_log->lock, NULL);
    pthread_key_create(&t_log->key, _user_thread_destroy);
    t_log->mode = LOG_SYNC_MODE;
    t_log->is_background_thread_started = 0;
    t_log->roll_size = LOG_DEFAULT_ROLL_SIZE;
    t_log->flush_interval = LOG_DEFAULT_FLUSH_INTERVAL;

    g_log = t_log;
    printf("log system init complete\n");
}

/******************************************************************************
 * Interfaces
 *****************************************************************************/
log_file_t* log_create(const char* filename){
    // init log system global data
    pthread_once(&init_create, _log_init);
    pthread_mutex_lock(&g_log->lock);

    // check whether log file data has been created
    // if not, create it, or return its pointer
    log_file_t* created_file = hash_get_str(g_log->phash, filename);
    if ( created_file ) {
        created_file->ref_count++;
        pthread_mutex_unlock(&g_log->lock);
        return created_file;
    }

    // the file struct is the first to create
    log_file_t* f = malloc(sizeof(log_file_t));
    if ( !f ) {
        printError("cannot alloc memory for log_file_t");
        pthread_mutex_unlock(&g_log->lock);
        return NULL;
    }

    // init log_file data
    _log_generate_filename(filename, f->poutput_filename);
    FILE* pf = _log_open(f->poutput_filename, f->filebuf,
                         LOG_BUFFER_SIZE_PER_FILE);
    if ( !pf ) {
        printError("open file failed");
        free(f);
        pthread_mutex_unlock(&g_log->lock);
        return NULL;
    }
    f->pf = pf;
    f->file_size = 0;
    f->last_flush_time = time(NULL);
    snprintf(f->pfilename, LOG_MAX_FILE_NAME, "%s", filename);
    hash_set_str(g_log->phash, filename, f);
    f->ref_count = 1;

    pthread_mutex_unlock(&g_log->lock);
    return f;
}

void log_destroy(log_file_t* lf)
{
    if ( !g_log || !lf ) return;
    pthread_mutex_lock(&g_log->lock);
    lf->ref_count--;

    if ( lf->ref_count == 0 ) {
        fclose(lf->pf);
        hash_del_str(g_log->phash, lf->pfilename);
        free(lf);
    }

    pthread_mutex_unlock(&g_log->lock);

}

/**
 * We have two ways to write a log message
 * 1. synchronization writing mode
 * 2. asynchronization writing mode
 *
 * @ When we use synchronization mode, it will call fwrite directly without 
 * buffer-queue
 * @ When we use asynchronization mode, it will push log message into thread
 * buffer, and notice the fetcher thread to fetch it
 */
size_t log_file_write(log_file_t* f, const char* log, size_t len)
{
    if ( !g_log || !f || !log || !len ) {
        printError("input invalid args");
        return 0;
    }

    if ( g_log->mode == LOG_ASYNC_MODE ) {
        return _log_async_write(f, log, len);
    } else {
        return _log_sync_write(f, log, len);
    }
}

LOG_MODE log_set_mode(LOG_MODE mode)
{
    if ( !g_log ) return LOG_SYNC_MODE;

    pthread_mutex_lock(&g_log->lock);
    if ( mode == g_log->mode ) {
        pthread_mutex_unlock(&g_log->lock);
        return g_log->mode;
    }

    if ( (mode == LOG_ASYNC_MODE) &&
         !g_log->is_background_thread_started ) {
        _create_fetcher_thread();
        g_log->is_background_thread_started = 1;
    }

    LOG_MODE last_mode = g_log->mode;
    g_log->mode = mode;
    pthread_mutex_unlock(&g_log->lock);
    return last_mode;
}

/**
 * If size == 0, that's a invalid argument
 */
void log_set_roll_size(size_t size)
{
    if ( !g_log || !size ) return;

    pthread_mutex_lock(&g_log->lock);
    g_log->roll_size = size;
    pthread_mutex_unlock(&g_log->lock);
}

/**
 * If sec == 0, it will flush it immediately
 */
void log_set_flush_interval(size_t sec)
{
    if ( !g_log ) return;

    pthread_mutex_lock(&g_log->lock);
    g_log->flush_interval = sec;
    pthread_mutex_unlock(&g_log->lock);
}
