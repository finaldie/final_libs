// create by final
// desc : test unit

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tu_inc.h"
#include "inc.h"

void	register_module(){
    tu_register_module(test_list, "for test flist");
    tu_register_module(test_hash, "for test fhash set and get");
    tu_register_module(test_hash_del, "for test hash_del method");
    tu_register_module(test_mem, "for test mempool alloc and free");
    tu_register_module(test_realloc, "for test mempool realloc");
    tu_register_module(test_log, "for test log system");
    tu_register_module(test_async_log, "for test log system");
    tu_register_module(test_mbuf, "for test mbuf of mbuf_seek & rewind & realloc");
    tu_register_module(test_mbuf1, "for test mbuf of mbuf_push & mbuf_pop");
    tu_register_module(test_timer, "for test ftimerfd");
    tu_register_module(test_fev, "for test fev for create register add del methods");
    tu_register_module(test_fev_listener, "for test fev listener");
    tu_register_module(test_fev_buff, "for test fev buff");
    tu_register_module(test_fev_conn, "for test fev asynchronous connect");
}

int main(int argc, char** argv){
    tu_register_init();
    register_module();
    tu_run_cases();

    return 0;
}
