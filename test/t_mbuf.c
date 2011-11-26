/*
 * =====================================================================================
 *
 *       Filename:  t_mbuf.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/25/2011 10:37:45
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */


#include "tu_inc.h"
#include "inc.h"
#include "mbuf.h"

void test_mbuf()
{
    mbuf* pbuf = mbuf_create(100);

    {
        int used = mbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(0, used);

        int total_free = mbuf_total_free(pbuf);
        FTU_ASSERT_EQUAL_INT(100, total_free);

        int head_free = mbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = mbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(100, tail_free);

        int size = mbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(100, size);
    }

    mbuf_head_seek(pbuf, 10);
    mbuf_tail_seek(pbuf, 20);

    {
        int used = mbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = mbuf_total_free(pbuf);
        FTU_ASSERT_EQUAL_INT(90, total_free);

        int head_free = mbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(10, head_free);

        int tail_free = mbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(80, tail_free);

        int size = mbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(100, size);
    }
    
    mbuf_rewind(pbuf);

    {
        int used = mbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = mbuf_total_free(pbuf);
        FTU_ASSERT_EQUAL_INT(90, total_free);

        int head_free = mbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = mbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(90, tail_free);

        int size = mbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(100, size);
    }

    mbuf* new_buf = mbuf_realloc(pbuf, 100);
    {
        FTU_ASSERT_EXPRESS((new_buf==pbuf));
    }

    pbuf = mbuf_realloc(new_buf, 200);

    {
        int used = mbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = mbuf_total_free(pbuf);
        FTU_ASSERT_EQUAL_INT(190, total_free);

        int head_free = mbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = mbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(190, tail_free);

        int size = mbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(200, size);
    }

    pbuf = mbuf_realloc(pbuf, 5);
    {
        int used = mbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = mbuf_total_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, total_free);

        int head_free = mbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = mbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, tail_free);

        int size = mbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);
    }

    pbuf = mbuf_realloc(pbuf, 100);
    mbuf_head_seek(pbuf, 10);
    mbuf_tail_seek(pbuf, 10);
    pbuf = mbuf_realloc(pbuf, 15);

    {
        int used = mbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = mbuf_total_free(pbuf);
        FTU_ASSERT_EQUAL_INT(5, total_free);

        int head_free = mbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = mbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(5, tail_free);

        int size = mbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(15, size);
    }

    mbuf_delete(pbuf);
}

void test_mbuf1()
{
    mbuf* pbuf = mbuf_create(100);
    FTU_ASSERT_EXPRESS(pbuf!=NULL);

    

    mbuf_delete(pbuf);
}
