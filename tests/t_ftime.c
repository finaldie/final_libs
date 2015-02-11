#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <fcunit.h>
#include "flibs/ftime.h"

void    test_ftimerfd()
{
    int fd = ftimerfd_create();
    FCUNIT_ASSERT(fd > 0);

    int ret = ftimerfd_start(fd, 1000000000l, 1000000000l);
    FCUNIT_ASSERT(0 == ret);
    sleep(2);

    uint64_t exp;
    ssize_t s = read(fd, (char*)&exp, sizeof(exp));
    FCUNIT_ASSERT(sizeof(exp) == s);

    ret = ftimerfd_stop(fd);
    FCUNIT_ASSERT(0 == ret);
}

int main(int argc, char** argv)
{
    FCUNIT_RUN(test_ftimerfd);

    return 0;
}
