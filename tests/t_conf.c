#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcunit.h>
#include "flibs/fconf.h"

#define TEST_CONF1 "tests/config/t1.conf"
#define TEST_CONF2 "tests/config/t2.conf"
#define TEST_CONF3 "tests/config/t3.conf"

void config_lookup(const char* key, const char* value)
{
    //printf("config key=[%s], value=[%s]\n", key, value);
}

void test_conf()
{
    //printf("loading config 1\n");
    FCUNIT_ASSERT(0 == fconf_load(TEST_CONF1, config_lookup));
    //printf("loading config 2\n");
    FCUNIT_ASSERT(1 == fconf_load(TEST_CONF2, config_lookup));
    //printf("loading config 3\n");
    FCUNIT_ASSERT(1 == fconf_load(TEST_CONF3, config_lookup));
}

int main(int argc, char** argv)
{
    FCUNIT_RUN(test_conf);

    return 0;
}
