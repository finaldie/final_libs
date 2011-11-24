
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>

#include "../base/lhash.h"
#include "tu_inc.h"

typedef unsigned char rpc_ver[17];

static
int		each(void* arg){
	int i=0;
	for(; i<16; ++i )
		printf("%x", ((unsigned char*)arg)[i]);
	printf("\n");

	return 0;
}

void	test_uuid(){
	f_hash* pmgr = hash_create(0);

	int j=0;
	for(; j<1; ++j){
		//uuid_t out;
		//uuid_generate(out);
		rpc_ver out;
		uuid_generate(out);
		out[16] = '\0';

		int i=0;
		for(; i<16; ++i )
			printf("%x\n", out[i]);
		printf("\n");

		hash_set_int(pmgr, j, (char*)out);
	}

	printf("-----------------------\n");
	hash_foreach(pmgr, each);
}
