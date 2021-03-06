#ifndef   _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200809L
#endif

#ifndef   _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "flibs/compiler.h"
#include "flibs/fmbuf.h"
#include "flibs/fhash.h"
#include "flibs/flog.h"

// BASE DEFINE
#define LOG_OPEN_PERMISSION           0644
#define LOG_COOKIE_MAX_LEN            (256)
#define LOG_BUFFER_SIZE_PER_FILE      (1024 * 128)
#define LOG_DEFAULT_ROLL_SIZE         (1024lu * 1024lu * 1024lu * 1lu)
#define LOG_DEFAULT_LOCAL_BUFFER_SIZE (1024 * 1024 * 10)
#define LOG_DEFAULT_FLUSH_INTERVAL    (0)
#define LOG_MAX_LEN_PER_MSG           (4096)
#define LOG_PTO_ID_SIZE               (sizeof(pto_id_t))
#define LOG_MSG_HEAD_SIZE             (sizeof(log_msg_head_t))

#define LOG_FETCHER_MAXPOLL           (1024)
#define LOG_FETCHER_MAXWAIT           (1000)
#define LOG_FETCHER_DEFAULT_NAME      ("logger")
#define LOG_NS_PER_MS                 (1000000L)

#define LOG_FILENAME_FMT              "%s-%s.%d"
#define LOG_FILE_TIME_FMT             "%04d_%02d_%02d_%02d_%02d_%02d.%03lu"
#define LOG_FILE_TIME_BUFSZ           (24)
#define LOG_MAX_FILE_NAME             (256)
#define LOG_MAX_OUTPUT_NAME           (512)

#define LOG_TIME_FMT                  "%04d-%02d-%02d %02d:%02d:%02d."
#define LOG_TIME_LEN                  (20)
#define LOG_TIME_MS_FMT               "%03lu "
#define LOG_TIME_MS_LEN               (4)
#define LOG_TIME_TOTAL_LEN            (LOG_TIME_LEN + LOG_TIME_MS_LEN)

// cookie
#define LOG_COOKIE_DEFAULT            "[] "
#define LOG_GET_COOKIE(header)        ((char*)header + LOG_TIME_TOTAL_LEN)

// debugging string
#define LOG_DEBUG_MAXLEN              (40)

// LOG PROTOCOL DEFINE
#define LOG_PTO_FETCH_MSG             0
#define LOG_PTO_THREAD_QUIT           1

// Atomic Interfaces
#define fatomic_inc(x) __sync_add_and_fetch(&(x), 1)

// Every file have only one log_file structure data
struct flog_file_t {
    size_t file_size;
    time_t last_flush_time;
    char   filename[LOG_MAX_FILE_NAME];
    char   poutput_filename[LOG_MAX_OUTPUT_NAME];
    size_t ref_count;
    size_t rolling_size;
    time_t flush_interval;

    uint32_t async :1; // Sync or Async
    uint32_t debug :1;
    uint32_t lv    :3;
    uint32_t _reserved :27;

    int    fd;        // File descriptor of logger file

    int    last_aid;  // Last auto id
    int    curr_aid;  // Current auto id
};

// Internal log message protocol

#pragma pack(1)
// Protocol id
typedef unsigned char pto_id_t;

// FetchMsg log header
typedef struct {
    flog_file_t*   f;
    int            aid;  // Auto incremental id
    unsigned short len;
} log_msg_head_t;

typedef struct {
    pto_id_t       id;
    log_msg_head_t msgh;
} log_fetch_msg_head_t;
#pragma pack()

// Every thread have a private thread_data for containing logbuf.
//  When write log messages, the data will be filled into this buffer
typedef struct _thread_data_t {
    fmbuf*       plog_buf;
    int          efd;        // eventfd, used for notify the async fetcher
    int          cookie_len;
    time_t       last_time;
    char         header[LOG_TIME_TOTAL_LEN + LOG_COOKIE_MAX_LEN + 3 + 1];
    char         last_time_str[LOG_TIME_LEN + 1];

    char         _padding[7];
} thread_data_t;

// Global log system data
typedef struct _log_t {
    fhash*          phash;       // mapping filename <--> log_file structure
    fhash*          threads;     // hold all threads (For Fetcher only)
    flog_event_func event_cb;    // event callback
    pthread_mutex_t lock;        // protect some scences resource competion
    pthread_key_t   key;

    int             epfd;        // epoll fd
    size_t          buffer_size; // buffer size per user thread
    pthread_t       b_tid;       // fetcher pthread id
    uint32_t        is_fetcher_started :1;
    uint32_t        is_fetcher_running :1;
    uint32_t        _reserved          :30;

    clockid_t       clockid;
} f_log;

// Define global log struct
static f_log* g_log = NULL;

// Ensure g_log only initialization once
static pthread_once_t init_create = PTHREAD_ONCE_INIT;

// Protocol table
typedef int (*ptofunc)(thread_data_t*);

static int _log_pto_fetch_msg  (thread_data_t* th_data);
static int _log_pto_thread_quit(thread_data_t* th_data);
static ptofunc g_pto_tbl[] = {
    _log_pto_fetch_msg,
    _log_pto_thread_quit
};

// Other internal api
static void _log_clear_cookie(thread_data_t* th_data);

#define printError(msg) \
    fprintf(stderr, "FATAL! Log system:[%s:%s(%d)] - errno=%d errmsg: %s\n",\
            __FILE__, __func__, __LINE__, errno, msg);

/******************************************************************************
 * Internal functions
 *****************************************************************************/
static
void _log_register_thread(int efd, thread_data_t* th_data) {
    fhash_int_set(g_log->threads, efd, th_data);
}

static
void _log_unregister_thread(int efd) {
    fhash_int_del(g_log->threads, efd);
}

static
void _destroy_thread_data(thread_data_t* th_data)
{
    if (!th_data) {
        return;
    }

    if (th_data->efd > 0) {
        // 1. De-register efd from epoll
        struct epoll_event ee;
        memset(&ee, 0, sizeof(ee));
        ee.events |= EPOLLIN;
        epoll_ctl(g_log->epfd, EPOLL_CTL_DEL, th_data->efd, &ee);

        // 2. Close efd
        close(th_data->efd);

        // 3. Unregister thread from global manager
        _log_unregister_thread(th_data->efd);

        // 4. Reset thread_data->efd
        th_data->efd = 0;
    }

    fmbuf_delete(th_data->plog_buf);
    th_data->plog_buf = NULL;

    free(th_data);
}

static inline
void _log_event_notice(flog_event_t event)
{
    if (g_log->event_cb) {
        g_log->event_cb(event);
    }
}

static
int _log_snprintf(char* dest, size_t max_len, const char* fmt, va_list ap)
{
    int copy_len = vsnprintf(dest, max_len, fmt ? fmt : "", ap);
    if (copy_len < 0) {
        // error
        printError("Fatal: _log_snprintf copy data ocurr errors");
        abort();
    } else if ((size_t)copy_len < max_len) {
        // copy successfully
    } else {
        // some data be truncated, but it shouldn't be happened, since we've
        // already checked it before call it
        printError("Fatal: _log_snprintf copy data was truncated");
        _log_event_notice(FLOG_EVENT_TRUNCATED);
        copy_len -= 1;
    }

    return copy_len;
}

static inline
int _log_set_nonblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag != -1) {
        return fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    }

    return flag;
}

static inline
bool _log_rolling_enabled(flog_file_t* f) {
    if (f->rolling_size && unlikely(f->file_size > f->rolling_size)) {
        return 1;
    }

    return 0;
}

static
size_t _log_filesize(flog_file_t* f)
{
    struct stat st;
    if (fstat(f->fd, &st)) {
        printError("cannot get metadata of log file");
        abort();
    }

    return (size_t)st.st_size;
}

static inline
int _lopen(const char* filename)
{
    return open(filename, O_CREAT | O_WRONLY | O_APPEND, LOG_OPEN_PERMISSION);
}

static inline
char* _log_generate_filename(const char* filename, char* output_filename)
{
    struct timespec tp;
    if (unlikely(clock_gettime(g_log->clockid, &tp))) {
        printError("cannot get current time");
        abort();
    }

    char now_time[LOG_FILE_TIME_BUFSZ];
    time_t tm_time = tp.tv_sec;
    struct tm now;
    gmtime_r(&tm_time, &now);
    snprintf(now_time, LOG_FILE_TIME_BUFSZ, LOG_FILE_TIME_FMT,
             (now.tm_year + 1900), now.tm_mon + 1, now.tm_mday,
             now.tm_hour, now.tm_min, now.tm_sec, tp.tv_nsec / LOG_NS_PER_MS);

    snprintf(output_filename, LOG_MAX_OUTPUT_NAME, LOG_FILENAME_FMT,
            filename, now_time, getpid());
    return output_filename;
}

// The time_str size > 21 at least
static
void _log_get_time(time_t tm_time, char* time_str)
{
    struct tm now;
    gmtime_r(&tm_time, &now);
    snprintf(time_str, LOG_TIME_LEN + 1, LOG_TIME_FMT,
             (now.tm_year+1900), now.tm_mon+1, now.tm_mday,
             now.tm_hour, now.tm_min, now.tm_sec);
}

// flush kernel data to disk
static
void _log_flush_file(flog_file_t* lf, time_t now)
{
    int r = fsync(lf->fd);
    lf->last_flush_time = now;

    if (r && lf->debug) {
        printError("cannot fsync file");
    }
}

static
void _log_roll_file(flog_file_t* lf)
{
    // 1. generate the rolled filename and move the current file to there
    _log_generate_filename(lf->filename, lf->poutput_filename);
    if (rename(lf->filename, lf->poutput_filename)) {
        printError("rotate file failed: call rename failed");
        abort();
    }
    close(lf->fd);

    // 2. open
    int fd = _lopen(lf->filename);
    if (fd < 0) {
        printError("open file failed");
        abort();
    }

    lf->fd = fd;
    lf->file_size = _log_filesize(lf);
}

static inline
size_t _log_write_to_kernel(flog_file_t* lf, const char* log, size_t len)
{
    if (!lf || !log || (len == 0)) {
        printError("cannot log any msg");
        return 0;
    }

    size_t remain_len = len;

    do {
        ssize_t real_len = write(lf->fd, log, len);
        if (real_len < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                printError("encounter error when writing logging message");
                return len - remain_len;
            }
        }

        remain_len -= (size_t)real_len;
    } while (remain_len);

    return len - remain_len;
}

static
thread_data_t* _log_create_thread_data()
{
    thread_data_t* th_data = calloc(1, sizeof (thread_data_t));

    fmbuf* pbuf = fmbuf_create(g_log->buffer_size);
    if (!pbuf) {
        printError("cannot alloc memory for thread data");
        _destroy_thread_data(th_data);
        return NULL;
    }
    th_data->plog_buf = pbuf;

    // Forward compatible with old version of kernel from 2.6.22 - 2.6.26
    //  we couldn't use EFD_NONBLOCK flag, so we need to call fcntl for
    //  setting nonblocking flag
    int efd = eventfd(0, 0);
    if (!efd) {
        printError("error in creating efd");
        _destroy_thread_data(th_data);
        return NULL;
    }

    if (_log_set_nonblocking(efd) == -1) {
        printError("error in set nonblocking flag for efd");
        _destroy_thread_data(th_data);
        return NULL;
    }

    if (efd == -1) {
        printError("cannot create eventfd for thread data");
        _destroy_thread_data(th_data);
        return NULL;
    }
    th_data->efd = efd;

    struct epoll_event ee;
    memset(&ee, 0, sizeof(ee));
    ee.data.ptr = th_data;
    ee.events |= EPOLLIN;
    if (epoll_ctl(g_log->epfd, EPOLL_CTL_ADD, efd, &ee)) {
        printError("cannot add eventfd into epoll for thread data");
        _destroy_thread_data(th_data);
        return NULL;
    }

    th_data->last_time = 0;
    _log_clear_cookie(th_data);

    return th_data;
}

static
thread_data_t* _get_or_create_thdata()
{
    thread_data_t* th_data = pthread_getspecific(g_log->key);
    if (likely(th_data)) {
        return th_data;
    }

    th_data = _log_create_thread_data();
    if (!th_data) {
        printError("Fatal: create thread data failed");
        abort();
    }

    if (pthread_setspecific(g_log->key, th_data) ) {
        printError("error in setting key");
        abort();
    }

    return th_data;
}

static
const char* _log_get_header(size_t* header_len/*out*/)
{
    thread_data_t* th_data = _get_or_create_thdata();

    struct timespec tp;
    if (unlikely(clock_gettime(g_log->clockid, &tp))) {
        printError("cannot get current time");
        abort();
    }

    time_t now = tp.tv_sec;
    if (now > th_data->last_time) {
        _log_get_time(now, th_data->last_time_str);
        th_data->last_time = now;
    }

    char tm_ms[LOG_TIME_MS_LEN + 1];
    snprintf(tm_ms, LOG_TIME_MS_LEN + 1, LOG_TIME_MS_FMT, tp.tv_nsec / LOG_NS_PER_MS);

    memcpy(th_data->header, th_data->last_time_str, LOG_TIME_LEN);
    memcpy(th_data->header + LOG_TIME_LEN, tm_ms, LOG_TIME_MS_LEN);

    *header_len = (size_t)(LOG_TIME_TOTAL_LEN + (size_t)th_data->cookie_len);
    return th_data->header;
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
size_t _log_async_write(flog_file_t* f,
                        const char* header, size_t header_len,
                        const char* log, size_t len)
{
    int is_report_truncated = 0;
    if (len > LOG_MAX_LEN_PER_MSG) {
        len = LOG_MAX_LEN_PER_MSG;
        is_report_truncated = 1;
    }

    // check pipe buffer whether have enough space
    thread_data_t* th_data = _get_or_create_thdata();
    size_t msg_body_len = header_len + len + 1;
    size_t total_msg_len = sizeof(log_fetch_msg_head_t) + msg_body_len;

    if (fmbuf_free(th_data->plog_buf) < total_msg_len) {
        _log_event_notice(FLOG_EVENT_BUFFER_FULL);
        return 0;
    }

    // wrap and push log message
    log_fetch_msg_head_t msg_header;
    msg_header.id       = LOG_PTO_FETCH_MSG;
    msg_header.msgh.f   = f;
    msg_header.msgh.aid = fatomic_inc(f->last_aid);
    msg_header.msgh.len = (unsigned short)msg_body_len;

    if (fmbuf_push(th_data->plog_buf, &msg_header,
                    sizeof(log_fetch_msg_head_t))) {
        _log_event_notice(FLOG_EVENT_ERROR_ASYNC_PUSH);
        return 0;
    }

    if (fmbuf_push(th_data->plog_buf, header, header_len)) {
        _log_event_notice(FLOG_EVENT_ERROR_ASYNC_PUSH);
        return 0;
    }

    if (fmbuf_push(th_data->plog_buf, log, len)) {
        _log_event_notice(FLOG_EVENT_ERROR_ASYNC_PUSH);
        return 0;
    }

    if (fmbuf_push(th_data->plog_buf, "\n", 1)) {
        _log_event_notice(FLOG_EVENT_ERROR_ASYNC_PUSH);
        return 0;
    }

    // notice fetcher to write log
    if (eventfd_write(th_data->efd, 1)) {
        _log_event_notice(FLOG_EVENT_ERROR_ASYNC_PUSH);
        return 0;
    }

    // this must be reported after async push
    if (is_report_truncated) {
        _log_event_notice(FLOG_EVENT_TRUNCATED);
    }

    return len;
}

static inline
size_t _log_fill_async_msg(flog_file_t* f, thread_data_t* th_data, char* buff,
                        const char* header, size_t header_len, const char* fmt,
                        va_list ap)
{
    char* tbuf = buff;
    log_fetch_msg_head_t* msg_header = (log_fetch_msg_head_t*)tbuf;
    msg_header->id       = LOG_PTO_FETCH_MSG;
    msg_header->msgh.f   = f;
    msg_header->msgh.aid = fatomic_inc(f->last_aid);
    msg_header->msgh.len = 0;

    tbuf += sizeof(log_fetch_msg_head_t);
    memcpy(tbuf, header, header_len);

    int debug_len = 0;
    if (unlikely(f->debug)) {
        debug_len = snprintf(tbuf + header_len, LOG_DEBUG_MAXLEN,
                                 "[aid=%d] ", msg_header->msgh.aid);

        if (debug_len < 0) {
            debug_len = 0;
            printError("copy debugging content failed");
        }
    }

    int copy_len = 0;
    copy_len = _log_snprintf(tbuf + header_len + debug_len,
                             LOG_MAX_LEN_PER_MSG - (size_t)debug_len + 1,
                             fmt, ap);

    // Fill \n
    if (copy_len > LOG_MAX_LEN_PER_MSG) {
        tbuf[header_len + (size_t)debug_len + LOG_MAX_LEN_PER_MSG] = '\n';
    } else {
        tbuf[header_len + (size_t)debug_len + (size_t)copy_len] = '\n';
    }

    // In the end, fill the size with real write number
    // Notes: the msg_len must be < 65535(unsigned short)
    size_t msg_len = header_len + (size_t)debug_len + (size_t)copy_len + 1;
    msg_header->msgh.len = (unsigned short)msg_len;

    return sizeof(log_fetch_msg_head_t) + msg_len;
}

static
void _log_async_write_f(flog_file_t* f,
                        const char* header, size_t header_len,
                        const char* fmt, va_list ap)
{
    // check whether have private thread data, if not, create it
    thread_data_t* th_data = _get_or_create_thdata();

    // check the pipe buffer whether have enough space
    size_t max_msg_len = sizeof(log_fetch_msg_head_t) + header_len +
                         LOG_MAX_LEN_PER_MSG + 1;

    size_t total_free = fmbuf_free(th_data->plog_buf);
    if (total_free < max_msg_len) {
        _log_event_notice(FLOG_EVENT_BUFFER_FULL);
        return;
    }

    char* head = fmbuf_head(th_data->plog_buf);
    char* tail = fmbuf_tail(th_data->plog_buf);
    size_t tail_free_size = tail < head ? total_free :
                                  fmbuf_tail_free(th_data->plog_buf);

    if (tail_free_size >= max_msg_len) {
        // fill log message in mbuf directly, prevent additional copying efforts
        char* buf = fmbuf_tail(th_data->plog_buf);
        size_t buff_size = _log_fill_async_msg(f, th_data, buf, header,
                                               header_len, fmt, ap);
        fmbuf_tail_seek(th_data->plog_buf, buff_size, FMBUF_SEEK_RIGHT);
    } else {
        // fill log message in tmp buffer
        char buf[max_msg_len];
        memset(buf, 0, max_msg_len);

        size_t buff_size = _log_fill_async_msg(f, th_data, buf, header,
                                               header_len, fmt, ap);
        fmbuf_push(th_data->plog_buf, buf, buff_size);
    }

    // notice fetcher to write log
    if (eventfd_write(th_data->efd, 1)) {
        _log_event_notice(FLOG_EVENT_ERROR_ASYNC_PUSH);
        return;
    }
}

static
void _log_try_flush_and_rollfile(flog_file_t* f)
{
    int has_flush = 0;
    time_t now = time(NULL);

    if (now - f->last_flush_time >= f->flush_interval) {
        _log_flush_file(f, now);
        has_flush = 1;
    }

    if (_log_rolling_enabled(f)) {
        // if not flush, force to flush once
        if (!has_flush) _log_flush_file(f, now);

        _log_roll_file(f);
    }
}

static
size_t _log_write(flog_file_t* f, const char* log, size_t len)
{
    size_t real_writen_len = _log_write_to_kernel(f, log, len);
    f->file_size += real_writen_len;

    _log_try_flush_and_rollfile(f);
    return real_writen_len;
}

static
size_t _log_sync_write_to_kernel(flog_file_t* f,
                             const char* header, size_t header_len,
                             const char* log, size_t len)
{
    size_t writen_len = 0;
    pthread_mutex_lock(&g_log->lock);
    {
        writen_len += _log_write(f, header, header_len);
        writen_len += _log_write(f, log, len);
        writen_len += _log_write(f, "\n", 1);
    }
    pthread_mutex_unlock(&g_log->lock);

    if (writen_len < len + header_len) {
        _log_event_notice(FLOG_EVENT_ERROR_WRITE);
    }

    return writen_len;
}

static
size_t _log_sync_write(flog_file_t* f,
                       const char* header, size_t header_len,
                       const char* log, size_t len)
{
    // The log will be truncated if the length > LOG_MAX_LEN_PER_MSG
    if (len > LOG_MAX_LEN_PER_MSG) {
        len = LOG_MAX_LEN_PER_MSG;
        _log_event_notice(FLOG_EVENT_TRUNCATED);
    }

    return _log_sync_write_to_kernel(f, header, header_len, log, len);
}

static
void _log_sync_write_f(flog_file_t* f,
                       const char* header, size_t header_len,
                       const char* fmt, va_list ap)
{
    char log[LOG_MAX_LEN_PER_MSG + 1];
    int len = _log_snprintf(log, LOG_MAX_LEN_PER_MSG + 1, fmt, ap);
    if (len < 0) {
        printError("sync write failed, due to _log_snprint return -1");
        abort();
    }

    _log_sync_write_to_kernel(f, header, header_len, log, (size_t)len);
}

static inline
int _log_pto_fetch_msg(thread_data_t* th_data)
{
    fmbuf* pbuf = th_data->plog_buf;

    char tmp_buf[LOG_MAX_LEN_PER_MSG];
    char* tmsg = NULL;
    log_fetch_msg_head_t header;
    size_t pbuf_sz = fmbuf_used(pbuf);
    size_t header_sz = sizeof(header);

    if (pbuf_sz < header_sz) {
        _log_event_notice(FLOG_EVENT_ERROR_ASYNC_POP);
        return 1;
    }

    // Fetch header
    memset(&header, 0, header_sz);

    log_fetch_msg_head_t* rheader =
        fmbuf_rawget(pbuf, &header, header_sz);
    header = *rheader;

    // Validate aid to decide that whether or not can log it right now
    if (header.msgh.aid != header.msgh.f->curr_aid + 1) {
        // wait for next round
        return 1;
    }

    header.msgh.f->curr_aid += 1;

    // Fetch log content
    size_t content_sz = (size_t)header.msgh.len;
    tmsg = fmbuf_rawget(pbuf, tmp_buf, header_sz + content_sz);
    if (!tmsg) {
        _log_event_notice(FLOG_EVENT_ERROR_ASYNC_POP);
        return 1;
    }

    size_t writen_len = _log_write(header.msgh.f, tmsg + header_sz, content_sz);

    if (writen_len < content_sz) {
        _log_event_notice(FLOG_EVENT_ERROR_WRITE);
    }

    fmbuf_pop(pbuf, NULL, header_sz + content_sz);
    return 0;
}

static inline
int _log_pto_thread_quit(thread_data_t* th_data)
{
    fmbuf_pop(th_data->plog_buf, NULL, LOG_PTO_ID_SIZE);

    _destroy_thread_data(th_data);
    _log_event_notice(FLOG_EVENT_USER_BUFFER_RELEASED);

    // Return 1 to avoid this event be counted
    return 1;
}

static inline
int _log_async_process(thread_data_t* th_data)
{
    int nprocessed = 0;
    for (;;) {
        // Check empty queue or not
        if (fmbuf_empty(th_data->plog_buf)) break;

        // First, pop protocol id
        pto_id_t id = 0;
        pto_id_t* rid = fmbuf_rawget(th_data->plog_buf, &id, LOG_PTO_ID_SIZE);
        id = *rid;

        // Run the protocol registration function from pto_tbl
        int ret = g_pto_tbl[id](th_data);
        if (ret) {
            break;
        } else {
            nprocessed++;
        }
    }

    return nprocessed;
}

static inline
int _log_process_timeout(void* ud,
                         const char* key,
                         void* value)
{
    flog_file_t* f = (flog_file_t*)value;
    time_t now = time(NULL);
    if (now - f->last_flush_time >= f->flush_interval) {
        _log_flush_file(f, now);
    }

    return 0;
}

static
void* _log_fetcher(void* arg __attribute__((unused))) {
    int nums, i, total_processed = 0;
    struct epoll_event events[LOG_FETCHER_MAXPOLL];
    g_log->is_fetcher_running = 1;

    do {
        nums = epoll_wait(g_log->epfd, events, LOG_FETCHER_MAXPOLL, LOG_FETCHER_MAXWAIT);
        if (nums < 0 && errno == EINTR) continue;

        // Timeout
        if (unlikely(nums == 0)) {
            pthread_mutex_lock(&g_log->lock);
            fhash_str_foreach(g_log->phash, _log_process_timeout, NULL);
            pthread_mutex_unlock(&g_log->lock);
        }

        // Process active threads
        for (i = 0; i < nums; i++) {
            struct epoll_event* ee = &events[i];
            thread_data_t* th_data = ee->data.ptr;
            uint64_t process_num = 0;

            // Register thread
            _log_register_thread(th_data->efd, th_data);

            // Process event
            if (eventfd_read(th_data->efd, &process_num)) {
                printError("error in read eventfd value");
                continue;
            }

            _log_async_process(th_data);
        }

        // Process all thread local buffers
        total_processed = 0;
        int round_processed = 0;

        do {
            fhash_int_iter iter = fhash_int_iter_new(g_log->threads);
            thread_data_t* th_data = NULL;

            round_processed = 0;
            while ((th_data = fhash_int_next(&iter))) {
                round_processed += _log_async_process(th_data);
            }

            fhash_int_iter_release(&iter);
            total_processed += round_processed;
        } while (round_processed);

    } while (likely(g_log->is_fetcher_running || nums || total_processed));

    return NULL;
}

static
void _create_fetcher_thread(){
    int rc = pthread_create(&g_log->b_tid, NULL, _log_fetcher, NULL);
    if (rc) {
        printError("create fetcher thread failed");
        exit(1);
    }

#ifdef _GNU_SOURCE
    pthread_setname_np(g_log->b_tid, LOG_FETCHER_DEFAULT_NAME);
#endif
}

static
void _user_thread_destroy(void* arg)
{
    // Note: DO NOT RELEASE thread data directly when thread buffer is not
    // empty, cause thread fetcher may using this data now.
    // The correct way is that transform responsibility of release from
    // user thread to fetcher, and the THREAD_QUIT msg will be the last msg of
    // the dying thread
    thread_data_t* th_data = (thread_data_t*)arg;
    if (!th_data) return;

    // Thread buffer is not empty, notice fetcher to release th_data
    pto_id_t id = LOG_PTO_THREAD_QUIT;
    fmbuf_push(th_data->plog_buf, &id, LOG_PTO_ID_SIZE);

    // Notice fetcher to fetch this exit message
    eventfd_write(th_data->efd, 1);
}

static
void _process_exit()
{
    // 1. wait fetcher quit
    g_log->is_fetcher_running = 0;
    pthread_join(g_log->b_tid, NULL);

    // 2. Release loggers
    fhash_str_iter iter = fhash_str_iter_new(g_log->phash);
    flog_file_t* logger = NULL;

    while ((logger = fhash_str_next(&iter))) {
        _log_flush_file(logger, 0);
        close(logger->fd);
        fhash_str_del(g_log->phash, logger->filename);
        free(logger);
    }

    fhash_str_iter_release(&iter);
}

static
void _log_init()
{
    f_log* t_log = calloc(1, sizeof(f_log));

    int epfd = epoll_create(1024);
    if (epfd == -1) {
        printError("cannot create epollfd for global log data");
        exit(1);
    }

    t_log->epfd    = epfd;
    t_log->phash   = fhash_str_create(0, FHASH_MASK_AUTO_REHASH);
    t_log->threads = fhash_int_create(0, FHASH_MASK_AUTO_REHASH);
    int rc = pthread_mutex_init(&t_log->lock, NULL);
    if (rc) {
        printError("cannot init global mutex");
        exit(1);
    }

    rc = pthread_key_create(&t_log->key, _user_thread_destroy);
    if (rc) {
        printError("cannot create pthread key");
        exit(1);
    }

    t_log->event_cb    = NULL;
    t_log->buffer_size = LOG_DEFAULT_LOCAL_BUFFER_SIZE;
    t_log->is_fetcher_started = 0;
    t_log->clockid = CLOCK_REALTIME;

    // Since we only care about the resolution in milliseconds level,
    //  to make it faster, try to get/set it to 'CLOCK_REALTIME_COARSE'
    //
    // Notes: In most of the system/device:
    //   - CLOCK_REALTIME / CLOCK_MONOTONIC has 1ns resolution
    //   - CLOCK_REALTIME_COARSE / CLOCK_MONOTONIC_COARSE has 1ms resolution
    //
    //   For some single-board unit, the xx_COARSE resolution could be 10ms,
    //   so only in this case we fallback to CLOCK_REALTIME.
    struct timespec resolution;
    if (0 == clock_getres(CLOCK_REALTIME_COARSE, &resolution)) {
        if (resolution.tv_nsec <= LOG_NS_PER_MS) {
            t_log->clockid = CLOCK_REALTIME_COARSE;
        }
    }

    g_log = t_log;

    rc = atexit(_process_exit);
    if (rc) {
        printError("cannot register the process exit function");
        exit(1);
    }
}

static inline
void _log_clear_cookie(thread_data_t* th_data)
{
    // reset cookie = "[] "
    char* cookie = LOG_GET_COOKIE(th_data->header);
    memcpy(cookie, LOG_COOKIE_DEFAULT, 4);

    // update cookie_len
    th_data->cookie_len = 3;
}

/******************************************************************************
 * Interfaces
 *****************************************************************************/
flog_file_t* flog_create(const char* filename, int flags)
{
    // init log system global data
    pthread_once(&init_create, _log_init);

    flog_file_t* f = NULL;
    pthread_mutex_lock(&g_log->lock);
    {
        // check whether log file data has been created
        // if not, create it, or return its pointer
        flog_file_t* created_file = fhash_str_get(g_log->phash, filename);
        if (created_file) {
            created_file->ref_count++;
            pthread_mutex_unlock(&g_log->lock);
            return created_file;
        }

        // the file struct is the first to create
        f = calloc(1, sizeof(flog_file_t));
        if (!f) {
            printError("cannot alloc memory for log_file_t");
            pthread_mutex_unlock(&g_log->lock);
            return NULL;
        }

        // init log_file data
        f->fd = _lopen(filename);
        if (f->fd < 0) {
            printError("open file failed");
            free(f);
            pthread_mutex_unlock(&g_log->lock);
            return NULL;
        }

        // set the file size according to the file's metadata, since we may
        // reopen it before it be rolled to another file
        f->file_size = _log_filesize(f);

        f->last_flush_time = time(NULL);
        snprintf(f->filename, LOG_MAX_FILE_NAME, "%s", filename);
        fhash_str_set(g_log->phash, filename, f);
        f->ref_count = 1;
        f->rolling_size = LOG_DEFAULT_ROLL_SIZE;
        f->flush_interval = LOG_DEFAULT_FLUSH_INTERVAL;
        f->async = (uint32_t)((flags & FLOG_F_ASYNC) != 0);
        f->debug = (uint32_t)((flags & FLOG_F_DEBUG) != 0);

        if (f->async && !g_log->is_fetcher_started) {
            g_log->is_fetcher_started = 1;
            _create_fetcher_thread();
        }
    }
    pthread_mutex_unlock(&g_log->lock);

    return f;
}

void flog_destroy(flog_file_t* lf)
{
    if (!g_log || !lf) return;
    pthread_mutex_lock(&g_log->lock);
    {
        lf->ref_count--;

        if (lf->ref_count == 0) {
            _log_flush_file(lf, 0);  // force to flush buffer to disk

            if (!lf->async) {
                close(lf->fd);
                fhash_str_del(g_log->phash, lf->filename);
                free(lf);
            }
        }
    }
    pthread_mutex_unlock(&g_log->lock);
}

/**
 * We have two ways to write a log message
 * 1. synchronization writing mode
 * 2. asynchronization writing mode
 *
 * @ When we use synchronization mode, it will call write directly without
 * buffer-queue
 * @ When we use asynchronization mode, it will push log message into thread
 * buffer, and notice the fetcher thread to fetch it
 */
size_t flog_write(flog_file_t* f, const char* log, size_t len)
{
    if (!g_log || !f) {
        printError("input invalid args");
        abort();
        return 1;
    }

    // update timestamp and return the header string
    size_t header_len = 0;
    const char* header = _log_get_header(&header_len);

    // write log according to the working mode
    if (unlikely(!f->async)) {
        return _log_sync_write(f, header, header_len, log, len);
    } else {
        return _log_async_write(f, header, header_len, log, len);
    }
}

void flog_writef(flog_file_t* f, const char* fmt, ...)
{
    if (!g_log || !f) {
        printError("input invalid args");
        abort();
        return;
    }

    // update timestamp and return the header string
    size_t header_len = 0;
    const char* header = _log_get_header(&header_len);

    va_list ap;
    va_start(ap, fmt);

    if (unlikely(!f->async)) {
        _log_sync_write_f(f, header, header_len, fmt, ap);
    } else {
        _log_async_write_f(f, header, header_len, fmt, ap);
    }

    va_end(ap);
}

void flog_vwritef(flog_file_t* f, const char* fmt, va_list ap)
{
    if (!g_log || !f) {
        printError("input invalid args");
        abort();
        return;
    }

    // update timestamp and return the header string
    size_t header_len = 0;
    const char* header = _log_get_header(&header_len);

    if (unlikely(!f->async)) {
        _log_sync_write_f(f, header, header_len, fmt, ap);
    } else {
        _log_async_write_f(f, header, header_len, fmt, ap);
    }
}

void flog_set_rolling_size(flog_file_t* logger, size_t size)
{
    if (!logger) return;

    pthread_mutex_lock(&g_log->lock);
    {
        logger->rolling_size = size;
    }
    pthread_mutex_unlock(&g_log->lock);
}

/**
 * If sec == 0, it will flush it immediately
 */
void flog_set_flush_interval(flog_file_t* logger, time_t sec)
{
    if (!logger) return;

    pthread_mutex_lock(&g_log->lock);
    {
        logger->flush_interval = sec;
    }
    pthread_mutex_unlock(&g_log->lock);
}

void flog_set_buffer_size(size_t size)
{
    // init log system global data
    pthread_once(&init_create, _log_init);
    if (!g_log) return;

    pthread_mutex_lock(&g_log->lock);
    {
        if (size == 0) size = LOG_DEFAULT_LOCAL_BUFFER_SIZE;
        size_t min_size = sizeof(log_fetch_msg_head_t) + LOG_MAX_LEN_PER_MSG;
        if (size < min_size) size = min_size;
        g_log->buffer_size = size;
    }
    pthread_mutex_unlock(&g_log->lock);
}

size_t flog_get_buffer_size()
{
    if (!g_log) return 0;
    return g_log->buffer_size;
}

void flog_register_event(flog_event_func pfunc)
{
    // init log system global data
    pthread_once(&init_create, _log_init);
    if (!g_log || !pfunc) return;

    pthread_mutex_lock(&g_log->lock);
    {
        g_log->event_cb = pfunc;
    }
    pthread_mutex_unlock(&g_log->lock);
}

// Set cookie string into per-thread cookie space, no need to lock here
void flog_set_cookie(const char* fmt, ...)
{
    // init log system global data
    pthread_once(&init_create, _log_init);

    // clear old cookie first
    thread_data_t* th_data = _get_or_create_thdata();
    flog_clear_cookie();

    // format and set the cookie
    va_list ap;
    va_start(ap, fmt);

    char* cookie = LOG_GET_COOKIE(th_data->header);
    cookie[0] = '[';
    int cookie_len = _log_snprintf(cookie + 1, LOG_COOKIE_MAX_LEN + 1,
                                   fmt, ap);
    cookie[cookie_len + 1] = ']';
    cookie[cookie_len + 2] = ' ';
    cookie_len += 3; // add the length of "[] "
    th_data->cookie_len = cookie_len;

    va_end(ap);
}

void flog_vset_cookie(const char* fmt, va_list ap)
{
    // init log system global data
    pthread_once(&init_create, _log_init);

    // clear the old cookie first
    thread_data_t* th_data = _get_or_create_thdata();
    flog_clear_cookie();

    // format and set the cookie
    char* cookie = LOG_GET_COOKIE(th_data->header);
    cookie[0] = '[';
    int cookie_len = _log_snprintf(cookie + 1, LOG_COOKIE_MAX_LEN + 1,
                                   fmt, ap);
    cookie[cookie_len + 1] = ']';
    cookie[cookie_len + 2] = ' ';
    cookie_len += 3; // add the length of "[] "
    th_data->cookie_len = cookie_len;
}

// Clear cookie string into per-thread cookie space, no need to lock here
void flog_clear_cookie()
{
    // init log system global data
    pthread_once(&init_create, _log_init);

    thread_data_t* th_data = _get_or_create_thdata();
    _log_clear_cookie(th_data);
}

int flog_set_level(flog_file_t* logger, int level) {
    if (!logger) return -1;

    int old_lv = logger->lv;
    logger->lv = (uint32_t)level & 0x7;

    return old_lv;
}

int flog_get_level(flog_file_t* logger) {
    return logger ? logger->lv : -1;
}

int flog_is_trace_enabled(flog_file_t* logger) {
    return logger ? FLOG_LEVEL_TRACE >= logger->lv : 0;
}

int flog_is_debug_enabled(flog_file_t* logger) {
    return logger ? FLOG_LEVEL_DEBUG >= logger->lv : 0;
}

int flog_is_info_enabled (flog_file_t* logger) {
    return logger ? FLOG_LEVEL_INFO >= logger->lv : 0;
}

int flog_is_warn_enabled (flog_file_t* logger) {
    return logger ? FLOG_LEVEL_WARN >= logger->lv : 0;
}

int flog_is_error_enabled(flog_file_t* logger) {
    return logger ? FLOG_LEVEL_ERROR >= logger->lv : 0;
}

int flog_is_fatal_enabled(flog_file_t* logger) {
    return logger ? FLOG_LEVEL_FATAL >= logger->lv : 0;
}

