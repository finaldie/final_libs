//base info: create by hyz
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "ftimer.h"
#include "ftu_inc.h"
#include "inc.h"

void    test_timer()
{
    int fd = ftimerfd_create();
    FTU_ASSERT_GREATER_THAN_INT(0, fd);

    int ret = ftimerfd_start(fd, 1000000000l, 1000000000l);
    FTU_ASSERT_EQUAL_INT(0, ret);
    sleep(2);

    uint64_t exp;
    int s = read(fd, (char*)&exp, sizeof(exp));
    FTU_ASSERT_EQUAL_INT((int)sizeof(exp), s);

    ret = ftimerfd_stop(fd);
    FTU_ASSERT_EQUAL_INT(0, ret);
}
