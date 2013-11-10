#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ftu_inc.h"
#include "inc.h"
#include "fread_conf.h"

#define TEST_CONF1 "./t1.conf"
#define TEST_CONF2 "./t2.conf"
#define TEST_CONF3 "./t3.conf"

void config_lookup(const char* key, const char* value)
{
    printf("config key=[%s], value=[%s]\n", key, value);
}

void test_conf()
{
    printf("loading config 1\n");
    FTU_ASSERT_EQUAL_INT(0, fload_config(TEST_CONF1, config_lookup));
    printf("loading config 2\n");
    FTU_ASSERT_EQUAL_INT(1, fload_config(TEST_CONF2, config_lookup));
    printf("loading config 3\n");
    FTU_ASSERT_EQUAL_INT(1, fload_config(TEST_CONF3, config_lookup));
}
