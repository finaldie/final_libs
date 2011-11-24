//base info: create by hyz
//effect:


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "../base/ltimer.h"
#include "tu_inc.h"

//TODO...

static
void	on_call(void* arg){
	printf("timer run...\n");
}

void	test_timer(){
	int fd = ftimerfd_create();
	
	ftimerfd_start(fd, 1000000000l, 1000000000l);

	uint64_t exp;
	while(1){
		int s = read(fd, (char*)&exp, sizeof(exp));

		if( s == sizeof(exp) )
			on_call(NULL);
	}
}


/*
int	main(int argc, char** argv)
{
	
	return 0;
}

*/
