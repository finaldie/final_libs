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
	tu_register_module("test_timer", test_timer);
}

int main(int argc, char** argv){
	tu_register_init();
	register_module();
	tu_run_cases();

	return 0;
}
