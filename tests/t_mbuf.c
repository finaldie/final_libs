/*
 * =============================================================================
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
 *         Author:  finaldie
 *        Company:
 *
 * =============================================================================
 */


#include "ftu/ftu_inc.h"
#include "fmbuf/fmbuf.h"
#include "inc.h"

// mbuf as a array
void test_mbuf()
{
    fmbuf* pbuf = fmbuf_create(100);

    {
        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(0, used);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(100, total_free);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(100, tail_free);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(100, size);
    }

    // setup mbuf, header at 10, tailer at 20 (10 bytes used)
    {
        fmbuf_head_seek(pbuf, 10, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 20, FMBUF_SEEK_RIGHT);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(90, total_free);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(10, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(80, tail_free);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(100, size);
    }

    // rewind it
    {
        fmbuf_rewind(pbuf);
        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(90, total_free);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(90, tail_free);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(100, size);
    }

    // realloc it with same size
    {
        fmbuf* new_buf = fmbuf_realloc(pbuf, 100);
        FTU_ASSERT_EXPRESS((new_buf==pbuf));
    }

    // increase the mbuf size
    {
        fmbuf* new_buf = pbuf;
        pbuf = fmbuf_realloc(new_buf, 200);
        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(190, total_free);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(190, tail_free);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(200, size);
    }

    // decrease the buff size < data size
    {
        pbuf = fmbuf_realloc(pbuf, 5);
        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, total_free);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, tail_free);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);
    }

    // realloc it, new buffer size > data size
    {
        pbuf = fmbuf_realloc(pbuf, 100);
        fmbuf_head_seek(pbuf, 10, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 10, FMBUF_SEEK_RIGHT);
        pbuf = fmbuf_realloc(pbuf, 15);
        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(5, total_free);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(5, tail_free);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(15, size);
    }

    fmbuf_delete(pbuf);
}

// mbuf as a ring-buffer (the tailer location < header location)
void test_mbuf2()
{
    // mbuf: size = 10b, used = 2b
    // realloc it with same size
    {
        fmbuf* mbuf = fmbuf_create(10);
        int ret = fmbuf_push(mbuf, "11", 2);
        FTU_ASSERT(ret == 0);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 10);
        FTU_ASSERT(new_buf == mbuf);
        fmbuf_delete(mbuf);
    }

    // mbuf: size = 10b, used = 9b
    // realloc it with new size = 20b
    // note: left used <= right used and left used <= increased sz
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 2, FMBUF_SEEK_RIGHT);
        FTU_ASSERT(fmbuf_used(mbuf) == 9);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 20);
        FTU_ASSERT(9 == fmbuf_used(new_buf));
        FTU_ASSERT(11 == fmbuf_free(new_buf));
        FTU_ASSERT(20 == fmbuf_size(new_buf));
        FTU_ASSERT(4 == fmbuf_head_free(new_buf));
        FTU_ASSERT(7 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 9b
    // realloc it with new size = 20b
    // note: left used > right used, left used <= increased sz
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 8, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 6, FMBUF_SEEK_RIGHT);
        FTU_ASSERT(fmbuf_used(mbuf) == 9);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 20);
        FTU_ASSERT(9 == fmbuf_used(new_buf));
        FTU_ASSERT(11 == fmbuf_free(new_buf));
        FTU_ASSERT(20 == fmbuf_size(new_buf));
        FTU_ASSERT(18 == fmbuf_head_free(new_buf));
        FTU_ASSERT(14 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 9b
    // realloc it with new size = 11b
    // note: left used <= right used, left used > increased sz
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 2, FMBUF_SEEK_RIGHT);
        FTU_ASSERT(fmbuf_used(mbuf) == 9);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 11);
        FTU_ASSERT(9 == fmbuf_used(new_buf));
        FTU_ASSERT(2 == fmbuf_free(new_buf));
        FTU_ASSERT(11 == fmbuf_size(new_buf));
        FTU_ASSERT(5 == fmbuf_head_free(new_buf));
        FTU_ASSERT(9 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 9b
    // realloc it with new size = 11b
    // note: left used > right used, left used > increased sz
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 8, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 6, FMBUF_SEEK_RIGHT);
        FTU_ASSERT(fmbuf_used(mbuf) == 9);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 11);
        FTU_ASSERT(9 == fmbuf_used(new_buf));
        FTU_ASSERT(2 == fmbuf_free(new_buf));
        FTU_ASSERT(11 == fmbuf_size(new_buf));
        FTU_ASSERT(9 == fmbuf_head_free(new_buf));
        FTU_ASSERT(5 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 2b
    // realloc it with new size = 5b
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 10, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 1, FMBUF_SEEK_RIGHT);
        FTU_ASSERT(fmbuf_used(mbuf) == 2);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 5);
        FTU_ASSERT(2 == fmbuf_used(new_buf));
        FTU_ASSERT(3 == fmbuf_free(new_buf));
        FTU_ASSERT(5 == fmbuf_size(new_buf));
        FTU_ASSERT(5 == fmbuf_head_free(new_buf));
        FTU_ASSERT(4 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 2b
    // realloc it with new size = 1b
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 10, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 1, FMBUF_SEEK_RIGHT);
        FTU_ASSERT(fmbuf_used(mbuf) == 2);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 1);
        FTU_ASSERT(2 == fmbuf_used(new_buf));
        FTU_ASSERT(0 == fmbuf_free(new_buf));
        FTU_ASSERT(2 == fmbuf_size(new_buf));
        FTU_ASSERT(2 == fmbuf_head_free(new_buf));
        FTU_ASSERT(1 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }
}

void test_mbuf1()
{
    // create a mbuf with size == 0
    {
        fmbuf* pbuf = fmbuf_create(0);
        FTU_ASSERT(pbuf != NULL);
        FTU_ASSERT(0 == fmbuf_size(pbuf));
        FTU_ASSERT(0 == fmbuf_used(pbuf));
        FTU_ASSERT(0 == fmbuf_free(pbuf));
        fmbuf_delete(pbuf);
    }

    fmbuf* pbuf = fmbuf_create(10);
    FTU_ASSERT_EXPRESS(pbuf!=NULL);

    char* push_buf[20];
    char* pop_buf[20];

    // push 1 byte data
    {
        int ret = fmbuf_push(pbuf, push_buf, 1);
        FTU_ASSERT_EQUAL_INT(0, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(1, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(9, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(9, total_free);
    }

    // push 9 byte data
    {
        int ret = fmbuf_push(pbuf, push_buf, 9);
        FTU_ASSERT_EQUAL_INT(0, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, total_free);
    }

    // continue push 1 byte data when mbuf is full
    {
        int ret = fmbuf_push(pbuf, push_buf, 1);
        FTU_ASSERT_EQUAL_INT(1, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, total_free);
    }

    // clear and seek mbuf and push 5 bytes
    {
        fmbuf_clear(pbuf);
        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(10, total_free);

        fmbuf_head_seek(pbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 6, FMBUF_SEEK_RIGHT);

        int ret = fmbuf_push(pbuf, push_buf, 5);
        FTU_ASSERT_EQUAL_INT(0, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(7, used);

        total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(3, total_free);
    }

    // push 4 bytes but the mbuf only have 3 bytes left
    {
        int ret = fmbuf_push(pbuf, push_buf, 4);
        FTU_ASSERT_EQUAL_INT(1, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(7, used);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(3, total_free);
    }

    // push 3 bytes the mbuf is full
    {
        int ret = fmbuf_push(pbuf, push_buf, 3);
        FTU_ASSERT_EQUAL_INT(0, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(10, used);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, total_free);

        ret = fmbuf_push(pbuf, push_buf, 1);
        FTU_ASSERT_EQUAL_INT(1, ret);
    }

    //---------------mbuf pop-------------------
    //pop 1 bytes
    {
        fmbuf_clear(pbuf);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int ret = fmbuf_pop(pbuf, pop_buf, 1);
        FTU_ASSERT_EQUAL_INT(1, ret);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(0, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(10, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(10, total_free);
    }

    // push 5 bytes and pop 1 bytes
    {
        int ret = fmbuf_push(pbuf, push_buf, 5);
        FTU_ASSERT_EQUAL_INT(0, ret);

        ret = fmbuf_pop(pbuf, pop_buf, 1);
        FTU_ASSERT_EQUAL_INT(0, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(4, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(1, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(5, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(6, total_free);
    }

    // pop 3 bytes and left 1 byte
    {
        int ret = fmbuf_pop(pbuf, pop_buf, 3);
        FTU_ASSERT_EQUAL_INT(0, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(1, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(4, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(5, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(9, total_free);
    }

    // pop 2 bytes
    {
        int ret = fmbuf_pop(pbuf, pop_buf, 2);
        FTU_ASSERT_EQUAL_INT(1, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(1, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(4, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(5, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(9, total_free);
    }

    // clear mbuf and seek tail < head
    {
        fmbuf_clear(pbuf);
        fmbuf_head_seek(pbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 2, FMBUF_SEEK_RIGHT);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(9, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(4, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(8, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(1, total_free);
    }

    // total 9 bytes, pop 7 bytes
    {
        int ret = fmbuf_pop(pbuf, pop_buf, 7);
        FTU_ASSERT_EQUAL_INT(0, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(2, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(0, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(8, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(8, total_free);
    }

    // pop 2 bytes and then the mbuf is empty
    {
        int ret = fmbuf_pop(pbuf, pop_buf, 2);
        FTU_ASSERT_EQUAL_INT(0, ret);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(0, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(2, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(8, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(10, total_free);
    }

    {
        fmbuf_clear(pbuf);
        fmbuf_head_seek(pbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 6, FMBUF_SEEK_RIGHT);

        int ret = fmbuf_pop(pbuf, NULL, 4);
        FTU_ASSERT(ret == 1);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(2, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(4, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(4, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(8, total_free);
    }

    // move 2 bytes
    {
        fmbuf_pop(pbuf, NULL, 2);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(0, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(6, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(4, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(10, total_free);
    }

    // tail < head, tail_use == 5, but pop 6 bytes
    {
        fmbuf_clear(pbuf);
        fmbuf_head_seek(pbuf, 6, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 4, FMBUF_SEEK_RIGHT);

        int used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(9, used);

        fmbuf_pop(pbuf, NULL, 6);

        int size = fmbuf_size(pbuf);
        FTU_ASSERT_EQUAL_INT(10, size);

        used = fmbuf_used(pbuf);
        FTU_ASSERT_EQUAL_INT(3, used);

        int head_free = fmbuf_head_free(pbuf);
        FTU_ASSERT_EQUAL_INT(1, head_free);

        int tail_free = fmbuf_tail_free(pbuf);
        FTU_ASSERT_EQUAL_INT(6, tail_free);

        int total_free = fmbuf_free(pbuf);
        FTU_ASSERT_EQUAL_INT(7, total_free);
    }

    fmbuf_delete(pbuf);
}
