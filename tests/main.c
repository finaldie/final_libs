// create by final
// desc : test unit

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ftu/ftu_inc.h"
#include "inc.h"

void register_module(){
    tu_register_module(test_hash_core,      "for testing fhash_core");
    tu_register_module(test_hash_int,       "for testing fhash_int");
    tu_register_module(test_hash_uint64,    "for testing fhash_uint64");
    tu_register_module(test_hash_str,       "for testing fhash_str");

    tu_register_module(test_conf,         "for testing fconf");
    tu_register_module(test_list,         "for testing flist");
    tu_register_module(test_list_sort,    "for testing flist_sort method");
    tu_register_module(test_mem,          "for testing mempool alloc and free");
    tu_register_module(test_realloc,      "for testing mempool realloc");
    tu_register_module(test_log,          "for testing log system");
    tu_register_module(test_async_log,    "for testing async log system");
    tu_register_module(test_mbuf,         "for testing mbuf of mbuf_seek & rewind & realloc");
    tu_register_module(test_mbuf1,        "for testing mbuf of mbuf_push & mbuf_pop");
    tu_register_module(test_timer,        "for testing ftimerfd");
    tu_register_module(test_fev,          "for testing fev for create register add del methods");
    tu_register_module(test_fev_listener, "for testing fev listener");
    tu_register_module(test_fev_buff,     "for testing fev buff");
    tu_register_module(test_fev_conn,     "for testing fev asynchronous connect");
    tu_register_module(test_fcache,       "for testing fcache set and get");
    tu_register_module(test_fco,          "for testing fco create, resume and yield");
    tu_register_module(test_timer_service,"for testing timer service");
}

int main(int argc    __attribute__((unused)),
         char** argv __attribute__((unused)))
{
    tu_register_init();
    register_module();
    int ret = tu_run_cases();
    exit(ret);

    return 0;
}
