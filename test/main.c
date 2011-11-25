// create by final
// desc : test unit

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tu_inc.h"
#include "inc.h"

void	register_module(){
	tu_register_module("test_hash", test_hash);
	tu_register_module("test_hash_del", test_hash_del);

	tu_register_module("test_alloc", test_mem);
	tu_register_module("test_realloc", test_realloc);
	tu_register_module("test_log", test_log);
	tu_register_module("test_mbuf", test_mbuf);
	//register_module("test_pto", t_pto);
	////register_module("test_mbuf", test_mbuf);
	//register_module("test_ploop", t_pto_loop);
	//register_module("test_press", test_pressure);
	//register_module("test_key", test_th_key);
	//register_module("test_timer", test_timer);
	//register_module("test_uuid", test_uuid);
	//register_module("test_auth", test_auth);
	//register_module("test_insert", test_insert);
	//register_module("test_bit", test_bit);
	//register_module("test_http", test_http);
	//register_module("test_host", test_host);
	//register_module("test_host1", test_host1);
	//register_module("test_host2", test_host2);
	//register_module("test_host3", test_host3);
	//register_module("test_strtok", test_strtok);
	//register_module("test_end", test_http_end);
	//register_module("test_getaddr", test_getaddr);
	//register_module("test_dns", test_dns);
	//register_module("test_localhost", test_local);
}

int main(int argc, char** argv){
	tu_register_init();
	register_module();
	tu_run_cases();

	return 0;
}
