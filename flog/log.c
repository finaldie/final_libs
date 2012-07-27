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

// BASE DEFINE
#define LOG_OPEN_PERMISSION           0644
#define LOG_MAX_FILE_NAME             (64)
#define LOG_MAX_OUTPUT_NAME           (128)
#define LOG_BUFFER_SIZE_PER_FILE      (1024 * 64)
#define LOG_DEFAULT_ROLL_SIZE         (1024 * 1024 * 1024)
#define LOG_DEFAULT_LOCAL_BUFFER_SIZE (1024 * 1024 * 10)
#define LOG_DEFAULT_FLUSH_INTERVAL    (0)
#define LOG_MAX_LEN_PER_MSG           (4096)
#define LOG_MSG_HEAD_SIZE             ( sizeof(log_msg_head_t) )
#define LOG_PTO_ID_SIZE               ( sizeof(pto_id_t) )
#define LOG_PTO_RESERVE_SIZE          LOG_PTO_ID_SIZE
#define LOG_TIME_STR_LEN              (21)

// LOG PROTOCOL DEFINE
#define LOG_PTO_FETCH_MSG          0
#define LOG_PTO_THREAD_QUIT        1

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

// internal log message protocol
#pragma pack(1)
// protocol id
typedef unsigned char pto_id_t;

// fetch log header
typedef struct {
    log_file_t*    f;
    unsigned short len;
}log_msg_head_t;

typedef struct {
    pto_id_t       id;
    log_msg_head_t msgh;
}log_fetch_msg_head_t;
#pragma pack()

// every thread have a private thread_data for containing logbuf
// when we write log messages, data will fill into this buffer
typedef struct _thread_data_t {
    mbuf*        plog_buf;
    int          efd;       // eventfd
    time_t       last_time;
    char         last_time_str[LOG_TIME_STR_LEN + 1];
    char         tmp_buf[LOG_MAX_LEN_PER_MSG];
}thread_data_t;

typedef void (*ptofunc)(thread_data_t*);

// global log system data
typedef struct _log_t {
    f_hash*         phash;       // mapping filename <--> log_file structure
    pthread_mutex_t lock;        // protect some scences resource competion
    pthread_key_t   key;
    plog_event_func event_cb;    // event callback
    LOG_MODE        mode;        // global log flag
    size_t          roll_size;
    size_t          buffer_size; // buffer size per user thread
    int             is_background_thread_started;
    int             flush_interval;
    int             epfd;        // epoll fd
    time_t          last_time;
    char            last_time_str[LOG_TIME_STR_LEN + 1];
}f_log;

// define global log struct
static f_log* g_log = NULL;

// ensure g_log only initialization once
static pthread_once_t init_create = PTHREAD_ONCE_INIT;

// protocol table
void _log_pto_fetch_msg  (thread_data_t* th_data);
void _log_pto_thread_quit(thread_data_t* th_data);
static ptofunc g_pto_tbl[] = {
    _log_pto_fetch_msg,
    _log_pto_thread_quit
};

#define printError(msg) \
    fprintf(stderr, "FATAL! Log system:[%s:%s(%d)] - errno=%d errmsg=%s:%s\n",\
            __FILE__, __func__, __LINE__, errno, strerror(errno), msg);

/******************************************************************************
 * Internal functions
 *****************************************************************************/
static inline
int _log_set_nonblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if ( flag != -1 ) {
        return fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    }

    return flag;
}

static inline
void _log_event_notice(LOG_EVENT event)
{
    if ( g_log->event_cb ) {
        g_log->event_cb(event);
    }
}

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

// The time_str size > 21 at least
static
void _log_get_time(time_t tm_time, char* time_str)
{
    struct tm now;
    gmtime_r(&tm_time, &now);
    snprintf(time_str, LOG_TIME_STR_LEN + 1, "[%04d-%02d-%02d %02d:%02d:%02d]",
                (now.tm_year+1900), now.tm_mon+1, now.tm_mday,
                now.tm_hour, now.tm_min, now.tm_sec);
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

static
thread_data_t* _log_create_thread_data()
{
    thread_data_t* th_data = malloc( sizeof (thread_data_t) );
    if ( !th_data ) {
        printError("cannot alloc memory for thread data");
        return NULL;
    }

    mbuf* pbuf = mbuf_create(g_log->buffer_size);
    if ( !pbuf ) {
        printError("cannot alloc memory for thread data");
        free(th_data);
        return NULL;
    }
    th_data->plog_buf = pbuf;

    // for forward compatible old version of kernel from 2.6.22 - 2.6.26
    // we couldn't use EFD_NONBLOCK flag, so we need to call fcntl for
    // setting nonblocking flag
    int efd = eventfd(0, 0);
    if ( _log_set_nonblocking(efd) == -1 ) {
        printError("error in set nonblocking flag for efd");
        free(th_data);
        return NULL;
    }

    if ( efd == -1 ) {
        printError("cannot create eventfd for thread data");
        free(th_data);
        return NULL;
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
        close(efd);
        free(th_data);
        return NULL;
    }

    th_data->last_time = 0;

    return th_data;
}

/**
 * Asynchronous mode we use a large ring buffer to hold the log messages data,
 * the format of log message is:
 * protocol_id(2 bytes) | data
 *
 * Currently, there are two protocols: 1) fetch msg, 2) thread quit
 * log_file_ptr(4/8 bytes) | len(2 bytes) | log message(len bytes)
 */
static
size_t _log_async_write(log_file_t* f, const char* file_sig, size_t sig_len,
                        const char* log, size_t len)
{
    if ( !f || !log || len == 0 ) {
        return 0;
    }

    // check whether have private thread data, if not, create it
    thread_data_t* th_data = pthread_getspecific(g_log->key);
    if ( !th_data ) {
        th_data = _log_create_thread_data();
        if ( !th_data ) return 0;
        if ( pthread_setspecific(g_log->key, th_data) ) {
            printError("error in setting key");
            close(th_data->efd);
            free(th_data);
            return 0;
        }
    }

    // wrap and push log message
    if ( len > LOG_MAX_LEN_PER_MSG) {
        _log_event_notice(LOG_EVENT_ERROR_MSG_SIZE);
        return 0;
    }

    if ( !file_sig ) sig_len = 0;
    size_t msg_body_len = LOG_TIME_STR_LEN + sig_len + len + 1;
    size_t total_msg_len = sizeof(log_fetch_msg_head_t) + msg_body_len;
    if ( mbuf_free(th_data->plog_buf) < (int)total_msg_len +
                                        LOG_PTO_RESERVE_SIZE ) {
        _log_event_notice(LOG_EVENT_BUFF_FULL);
        return 0;
    }

    log_fetch_msg_head_t header;
    header.id = LOG_PTO_FETCH_MSG;
    header.msgh.f = f;
    header.msgh.len = (unsigned short)msg_body_len;
    if ( mbuf_push(th_data->plog_buf, &header, sizeof(log_fetch_msg_head_t)) ) {
        return 0;
    }

    time_t now = time(NULL);
    if ( now > th_data->last_time ) {
        _log_get_time(now, th_data->last_time_str);
        th_data->last_time = now;
    }

    if( mbuf_push(th_data->plog_buf, th_data->last_time_str, LOG_TIME_STR_LEN) ) {
        return 0;
    }

    if ( file_sig && sig_len && mbuf_push(th_data->plog_buf, file_sig, sig_len) ) {}

    if( mbuf_push(th_data->plog_buf, log, len) ) {
        return 0;
    }

    if( mbuf_push(th_data->plog_buf, "\n", 1) ) {
        return 0;
    }

    // notice fetcher to write log
    if ( eventfd_write(th_data->efd, 1) ) {
        return 0;
    }

    return len;
}

static inline
int _log_fill_async_msg(log_file_t* f, thread_data_t* th_data, char* buff,
                        const char* file_sig, size_t sig_len, const char* fmt,
                        va_list ap)
{
    char* tbuf = buff;
    log_fetch_msg_head_t* header = (log_fetch_msg_head_t*)tbuf;
    header->id = LOG_PTO_FETCH_MSG;
    header->msgh.f = f;
    header->msgh.len = 0;

    time_t now = time(NULL);
    if ( now > th_data->last_time ) {
        _log_get_time(now, th_data->last_time_str);
        th_data->last_time = now;
    }

    tbuf += sizeof(log_fetch_msg_head_t);
    memcpy(tbuf, th_data->last_time_str, LOG_TIME_STR_LEN);

    if ( !file_sig ) sig_len = 0;
    else memcpy(tbuf + LOG_TIME_STR_LEN, file_sig, sig_len);

    int copy_len = 0;
    int head_len = LOG_TIME_STR_LEN + sig_len;
    int max_len = LOG_MAX_LEN_PER_MSG - head_len - 1;
    copy_len = vsnprintf(tbuf + head_len, max_len, fmt, ap);

    // fill \n
    if ( copy_len > max_len ) {
        tbuf[LOG_MAX_LEN_PER_MSG - 1] = '\n';
    } else {
        tbuf[head_len + copy_len] = '\n';
    }

    // after all, fill the size with real write number
    int msg_len = head_len + copy_len + 1;
    header->msgh.len = (unsigned short)msg_len;

    return (int)sizeof(log_fetch_msg_head_t) + msg_len;
}

static
void _log_async_write_f(log_file_t* f, const char* file_sig, size_t sig_len,
                        const char* fmt, va_list ap)
{
    if ( !f || !fmt ) return;

    // check whether have private thread data, if not, create it
    thread_data_t* th_data = pthread_getspecific(g_log->key);
    if ( !th_data ) {
        th_data = _log_create_thread_data();
        if ( !th_data ) return;
        if ( pthread_setspecific(g_log->key, th_data) ) {
            printError("error in setting key");
            close(th_data->efd);
            free(th_data);
            return;
        }
    }

    size_t max_msg_len = sizeof(log_fetch_msg_head_t) + LOG_MAX_LEN_PER_MSG +
                        LOG_PTO_RESERVE_SIZE;
    size_t total_free = mbuf_free(th_data->plog_buf);
    if ( total_free < max_msg_len ) {
        _log_event_notice(LOG_EVENT_BUFF_FULL);
        return;
    }

    char* head = mbuf_get_head(th_data->plog_buf);
    char* tail = mbuf_get_tail(th_data->plog_buf);
    int tail_free_size = tail < head ? total_free :
                                  mbuf_tail_free(th_data->plog_buf);
    if ( tail_free_size >= (int)max_msg_len ) {
        // fill log message in mbuf directly
        char* buf = mbuf_get_tail(th_data->plog_buf);
        int buff_size = _log_fill_async_msg(f, th_data, buf, file_sig, sig_len,
                                            fmt, ap);
        mbuf_tail_seek(th_data->plog_buf, buff_size);
    } else {
        // fill log message in tmp buffer
        char* buf = th_data->tmp_buf;
        int buff_size = _log_fill_async_msg(f, th_data, buf, file_sig, sig_len,
                                            fmt, ap);
        mbuf_push(th_data->plog_buf, buf, buff_size);
    }

    // notice fetcher to write log
    if ( eventfd_write(th_data->efd, 1) ) {
        return;
    }
}

static inline
size_t _log_write(log_file_t* f, const char* log, size_t len)
{
    size_t real_writen_len = _log_write_unlocked(f, log, len);

    int has_flush = 0;
    f->file_size += real_writen_len;
    time_t now = time(NULL);
    if ( now - f->last_flush_time >= g_log->flush_interval ) {
        _log_flush_file(f, now);
        has_flush = 1;
    }

    if ( f->file_size > g_log->roll_size ) {
        // if not flush, force to flush once
        if ( !has_flush ) _log_flush_file(f, now);
        _log_roll_file(f);
    }

    return real_writen_len;
}

static inline
size_t _log_wrap_sync_head(char* buf, const char* file_sig, size_t sig_len)
{
    time_t now = time(NULL);
    if ( now > g_log->last_time ) {
        do {
            pthread_mutex_lock(&g_log->lock);
            time_t check_time = time(NULL);
            if ( now != check_time ) {
                pthread_mutex_unlock(&g_log->lock);
                break;
            }

            _log_get_time(now, g_log->last_time_str);
            g_log->last_time = now;
            pthread_mutex_unlock(&g_log->lock);
        } while (0);
    }
    memcpy(buf, g_log->last_time_str, LOG_TIME_STR_LEN);

    if ( !file_sig ) sig_len = 0;
    else memcpy(buf + LOG_TIME_STR_LEN, file_sig, sig_len);

    return LOG_TIME_STR_LEN + sig_len;
}

static
size_t _log_sync_write(log_file_t* f, const char* file_sig, size_t sig_len,
                        const char* log, size_t len)
{
    char buf[LOG_MAX_LEN_PER_MSG];
    size_t head_len = _log_wrap_sync_head(buf, file_sig, sig_len);
    size_t writen_len = 0;
    pthread_mutex_lock(&g_log->lock);
    {
        writen_len += _log_write(f, buf, head_len);
        writen_len += _log_write(f, log, len);
        writen_len += _log_write(f, "\n", 1);
    }
    pthread_mutex_unlock(&g_log->lock);

    if ( writen_len < len + head_len ) {
        _log_event_notice(LOG_EVENT_ERROR_WRITE);
    }

    return writen_len;
}

static
void _log_sync_write_f(log_file_t* f, const char* file_sig, size_t sig_len,
                        const char* fmt, va_list ap)
{
    if ( !f || !fmt ) return;

    char log[LOG_MAX_LEN_PER_MSG];
    int copy_len = 0;
    size_t writen_len = 0;
    size_t head_len = _log_wrap_sync_head(log, file_sig, sig_len);
    size_t max_len = LOG_MAX_LEN_PER_MSG - head_len - 1;
    copy_len = vsnprintf(log + head_len, max_len, fmt, ap);

    // fill \n
    if ( copy_len > (int)max_len ) {
        log[LOG_MAX_LEN_PER_MSG - 1] = '\n';
    } else {
        log[head_len + copy_len] = '\n';
    }

    int len = head_len + copy_len + 1;
    pthread_mutex_lock(&g_log->lock);
    {
        writen_len = _log_write(f, log, len);
    }
    pthread_mutex_unlock(&g_log->lock);

    if ( writen_len < len ) {
        _log_event_notice(LOG_EVENT_ERROR_WRITE);
    }
}

inline
void _log_pto_fetch_msg(thread_data_t* th_data)
{
    mbuf* pbuf = th_data->plog_buf;
    char tmp_buf[LOG_MAX_LEN_PER_MSG];
    char* tmsg = NULL;
    log_msg_head_t header;

    if ( mbuf_pop(pbuf, &header, LOG_MSG_HEAD_SIZE) ) {
        return;
    }

    tmsg = mbuf_getraw(pbuf, tmp_buf, (size_t)header.len);
    if ( !tmsg ) {
        return;
    }

    size_t writen_len = _log_write(header.f, tmsg, (size_t)header.len);
    if ( writen_len < (size_t)header.len) {
        _log_event_notice(LOG_EVENT_ERROR_WRITE);
    }

    mbuf_head_move(pbuf, (size_t)header.len);
}

inline
void _log_pto_thread_quit(thread_data_t* th_data)
{
    if ( th_data->plog_buf )
        mbuf_delete(th_data->plog_buf);
    close(th_data->efd);
    free(th_data);
    _log_event_notice(LOG_EVENT_USER_BUFFER_RELEASED);
}

static inline
void _log_async_process(thread_data_t* th_data, unsigned int process_num)
{
    unsigned int i;
    for (i=0; i<process_num; i++) {
        // first, pop protocol id
        pto_id_t id = 0;
        if ( mbuf_pop(th_data->plog_buf, &id, LOG_PTO_ID_SIZE) ) {
            break;
        }

        g_pto_tbl[id](th_data);
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
    pthread_detach(pthread_self());

LOG_LOOP:
    nums = epoll_wait(g_log->epfd, events, 1024, 1000);
    if ( nums < 0 && errno == EINTR )
        goto LOG_LOOP;

    // timeout
    if ( nums == 0 ) {
        pthread_mutex_lock(&g_log->lock);
        hash_foreach(g_log->phash, _log_process_timeout);
        pthread_mutex_unlock(&g_log->lock);
    }

    for(i=0; i<nums; i++) {
        struct epoll_event* ee = &events[i];
        thread_data_t* th_data = ee->data.ptr;
        uint64_t process_num = 0;

        if ( eventfd_read(th_data->efd, &process_num) ) {
            printError("error in read eventfd value");
            continue;
        }

        _log_async_process(th_data, process_num);
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
    // note: DO NOT RELEASE thread data directly when thread buffer is not
    // empty, cause thread fetcher may using this data now.
    // The correct way is that transform responsibility of release from
    // user thread to fetcher
    thread_data_t* th_data = (thread_data_t*)arg;
    if ( !th_data ) return;

    if ( mbuf_used(th_data->plog_buf) == 0 ) {
        // thread buffer is empty
        if ( th_data->plog_buf )
            mbuf_delete(th_data->plog_buf);
        close(th_data->efd);
        free(th_data);
        _log_event_notice(LOG_EVENT_USER_BUFFER_RELEASED);
    } else {
        // thread buffer is no empty, notice fetcher to release th_data
        pto_id_t id = LOG_PTO_THREAD_QUIT;
        mbuf_push(th_data->plog_buf, &id, LOG_PTO_ID_SIZE);
    }
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
    t_log->event_cb = NULL;
    t_log->mode = LOG_SYNC_MODE;
    t_log->roll_size = LOG_DEFAULT_ROLL_SIZE;
    t_log->buffer_size = LOG_DEFAULT_LOCAL_BUFFER_SIZE;
    t_log->is_background_thread_started = 0;
    t_log->flush_interval = LOG_DEFAULT_FLUSH_INTERVAL;
    t_log->last_time = 0;

    g_log = t_log;
    printf("log system init complete\n");
}

/******************************************************************************
 * Interfaces
 *****************************************************************************/
log_file_t* log_create(const char* filename){
    // init log system global data
    pthread_once(&init_create, _log_init);

    log_file_t* f = NULL;
    pthread_mutex_lock(&g_log->lock);
    {
        // check whether log file data has been created
        // if not, create it, or return its pointer
        log_file_t* created_file = hash_get_str(g_log->phash, filename);
        if ( created_file ) {
            created_file->ref_count++;
            pthread_mutex_unlock(&g_log->lock);
            return created_file;
        }

        // the file struct is the first to create
        f = malloc(sizeof(log_file_t));
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
    }
    pthread_mutex_unlock(&g_log->lock);
    return f;
}

void log_destroy(log_file_t* lf)
{
    if ( !g_log || !lf ) return;
    pthread_mutex_lock(&g_log->lock);
    {
        lf->ref_count--;

        if ( lf->ref_count == 0 ) {
            fclose(lf->pf);
            hash_del_str(g_log->phash, lf->pfilename);
            free(lf);
        }
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
size_t log_file_write(log_file_t* f, const char* file_sig, size_t sig_len,
                        const char* log, size_t len)
{
    if ( !g_log || !f || !log || !len ) {
        printError("input invalid args");
        return 0;
    }

    if ( g_log->mode == LOG_ASYNC_MODE ) {
        return _log_async_write(f, file_sig, sig_len, log, len);
    } else {
        return _log_sync_write(f, file_sig, sig_len, log, len);
    }
}

void log_file_write_f(log_file_t* f, const char* file_sig, size_t sig_len,
                        const char* fmt, ...)
{
    if ( !g_log ) return;
    va_list ap;
    va_start(ap, fmt);

    if ( g_log->mode == LOG_ASYNC_MODE ) {
        _log_async_write_f(f, file_sig, sig_len, fmt, ap);
    } else {
        _log_sync_write_f(f, file_sig, sig_len, fmt, ap);
    }

    va_end(ap);
}

LOG_MODE log_set_mode(LOG_MODE mode)
{
    // init log system global data
    pthread_once(&init_create, _log_init);
    if ( !g_log ) return LOG_SYNC_MODE;

    LOG_MODE last_mode;
    pthread_mutex_lock(&g_log->lock);
    {
        if ( mode == g_log->mode ) {
            pthread_mutex_unlock(&g_log->lock);
            return g_log->mode;
        }

        if ( (mode == LOG_ASYNC_MODE) &&
             !g_log->is_background_thread_started ) {
            _create_fetcher_thread();
            g_log->is_background_thread_started = 1;
        }

        last_mode = g_log->mode;
        g_log->mode = mode;
    }
    pthread_mutex_unlock(&g_log->lock);
    return last_mode;
}

/**
 * If size == 0, that's a invalid argument
 */
void log_set_roll_size(size_t size)
{
    // init log system global data
    pthread_once(&init_create, _log_init);
    if ( !g_log || !size ) return;

    pthread_mutex_lock(&g_log->lock);
    {
        g_log->roll_size = size;
    }
    pthread_mutex_unlock(&g_log->lock);
}

/**
 * If sec == 0, it will flush it immediately
 */
void log_set_flush_interval(size_t sec)
{
    // init log system global data
    pthread_once(&init_create, _log_init);
    if ( !g_log ) return;

    pthread_mutex_lock(&g_log->lock);
    {
        g_log->flush_interval = sec;
    }
    pthread_mutex_unlock(&g_log->lock);
}

void log_set_buffer_size(size_t size)
{
    // init log system global data
    pthread_once(&init_create, _log_init);
    if ( !g_log ) return;

    pthread_mutex_lock(&g_log->lock);
    {
        if ( size == 0 ) size = LOG_DEFAULT_LOCAL_BUFFER_SIZE;
        size_t min_size = sizeof(log_fetch_msg_head_t) + LOG_MAX_LEN_PER_MSG +
                            LOG_PTO_RESERVE_SIZE;
        if ( size < min_size ) size = min_size;
        g_log->buffer_size = size;
    }
    pthread_mutex_unlock(&g_log->lock);
}

size_t log_get_buffer_size()
{
    if ( !g_log ) return 0;
    return g_log->buffer_size;
}

void log_register_event_callback(plog_event_func pfunc)
{
    // init log system global data
    pthread_once(&init_create, _log_init);
    if ( !g_log || !pfunc ) return;

    pthread_mutex_lock(&g_log->lock);
    {
        g_log->event_cb = pfunc;
    }
    pthread_mutex_unlock(&g_log->lock);
}
