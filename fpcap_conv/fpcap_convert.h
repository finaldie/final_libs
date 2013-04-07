/*
 * =====================================================================================
 *
 *       Filename:  pcap_convert.h
 *
 *    Description:  The framework of convertion
 *
 *        Version:  1.0
 *        Created:  03/17/2013 14:55:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  finaldie
 *
 * =====================================================================================
 */

#ifndef _PCAP_CONVERT_H_
#define _PCAP_CONVERT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FPCAP_CONV_CLIENT = 0x1,
    FPCAP_CONV_SERVER = 0x2
} FPCAP_CONV_TYPE;

typedef struct {
    struct timeval ts;
    char*  data;
    int    len;
} fapp_data_t;

typedef struct session_t {
    uint64_t id;
    void*    ud; // this ud for better to save some data, but user take the free responsibility
} session_t;

typedef enum {
    FSESSION_CREATE,
    FSESSION_DELETE,
    FSESSION_PROCESS
} fsession_event;

typedef void (*fconv_handler)(fsession_event, session_t*, fapp_data_t*, void* ud);
typedef int (*fsession_loop)(session_t*, void* ud);

typedef struct convert_action_t {
    const char*     pcap_filename;
    const char*     filter_rules;
    uint32_t        type; //convertion type:CLIENT, SERVER, CLIENT|SERVER
    fconv_handler   handler;
    fsession_loop   cleanup;
    void*           ud;
} convert_action_t;

int fpcap_convert(convert_action_t);

#ifdef __cplusplus
}
#endif

#endif
