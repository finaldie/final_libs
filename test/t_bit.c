// test bit operation
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "tu_inc.h"

static inline
int		lnet_get_event_type(uint64_t data){
	//return ((check_head*)&data)->type;
	return (int)(((data & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF);
}

static inline
int		lnet_get_event_fd(uint64_t data){
	//return ((check_head*)&data)->vfd;
	return (int)( data & 0xFFFFFFFF );
}

static inline
uint64_t	lnet_make_eventdata(int event_type, int vfd)
{
	uint64_t event_data = 0;
	event_data |= ((uint64_t)event_type << 32) & 0xFFFFFFFF00000000;
	event_data |= ((uint64_t)vfd) & 0xFFFFFFFF;

	return event_data;
}

void	test_bit()
{
	uint64_t a = lnet_make_eventdata(1, 1);
	int vfd = lnet_get_event_fd(a);
	int type = lnet_get_event_type(a);
	printf("a=0x%lX type=%d vfd=%d \n", a, type, vfd);

	a = lnet_make_eventdata(2, 1);
	vfd = lnet_get_event_fd(a);
	type = lnet_get_event_type(a);
	printf("a=0x%lX type=%d vfd=%d \n", a, type, vfd);

	a = lnet_make_eventdata(1, 2);
	vfd = lnet_get_event_fd(a);
	type = lnet_get_event_type(a);
	printf("a=0x%lX type=%d vfd=%d \n", a, type, vfd);


	a = lnet_make_eventdata(1, 0);
	vfd = lnet_get_event_fd(a);
	type = lnet_get_event_type(a);
	printf("a=0x%lX type=%d vfd=%d \n", a, type, vfd);

	a = lnet_make_eventdata(0, 1);
	vfd = lnet_get_event_fd(a);
	type = lnet_get_event_type(a);
	printf("a=0x%lX type=%d vfd=%d \n", a, type, vfd);
}
