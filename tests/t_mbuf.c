#include <fcunit.h>
#include "flibs/fmbuf.h"

// test fmbuf(array) seek/rewind/realloc
void test_mbuf()
{
    fmbuf* pbuf = fmbuf_create(100);

    {
        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(0 == used);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(100 == total_free);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(100 == tail_free);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(100 == size);
    }

    // setup mbuf, header at 10, tailer at 20 (10 bytes used)
    {
        fmbuf_head_seek(pbuf, 10, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 20, FMBUF_SEEK_RIGHT);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(10 == used);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(90 == total_free);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(10 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(80 == tail_free);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(100 == size);
    }

    // rewind it
    {
        fmbuf_rewind(pbuf);
        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(10 == used);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(90 == total_free);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(90 == tail_free);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(100 == size);
    }

    // realloc it with same size
    {
        fmbuf* new_buf = fmbuf_realloc(pbuf, 100);
        FCUNIT_ASSERT((new_buf==pbuf));
    }

    // increase the mbuf size
    {
        fmbuf* new_buf = pbuf;
        pbuf = fmbuf_realloc(new_buf, 200);
        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(10 == used);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(190 == total_free);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(190 == tail_free);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(200 == size);
    }

    // decrease the buff size < data size
    {
        pbuf = fmbuf_realloc(pbuf, 5);
        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(10 == used);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(0 == total_free);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(0 == tail_free);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);
    }

    // realloc it, new buffer size > data size
    {
        pbuf = fmbuf_realloc(pbuf, 100);
        fmbuf_head_seek(pbuf, 10, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 10, FMBUF_SEEK_RIGHT);
        pbuf = fmbuf_realloc(pbuf, 15);
        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(10 == used);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(5 == total_free);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(5 == tail_free);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(15 == size);
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
        FCUNIT_ASSERT(ret == 0);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 10);
        FCUNIT_ASSERT(new_buf == mbuf);
        fmbuf_delete(mbuf);
    }

    // mbuf: size = 10b, used = 9b
    // realloc it with new size = 20b
    // note: left used <= right used and left used <= increased sz
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 2, FMBUF_SEEK_RIGHT);
        FCUNIT_ASSERT(fmbuf_used(mbuf) == 9);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 20);
        FCUNIT_ASSERT(9 == fmbuf_used(new_buf));
        FCUNIT_ASSERT(11 == fmbuf_free(new_buf));
        FCUNIT_ASSERT(20 == fmbuf_size(new_buf));
        FCUNIT_ASSERT(4 == fmbuf_head_free(new_buf));
        FCUNIT_ASSERT(7 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 9b
    // realloc it with new size = 20b
    // note: left used > right used, left used <= increased sz
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 8, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 6, FMBUF_SEEK_RIGHT);
        FCUNIT_ASSERT(fmbuf_used(mbuf) == 9);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 20);
        FCUNIT_ASSERT(9 == fmbuf_used(new_buf));
        FCUNIT_ASSERT(11 == fmbuf_free(new_buf));
        FCUNIT_ASSERT(20 == fmbuf_size(new_buf));
        FCUNIT_ASSERT(18 == fmbuf_head_free(new_buf));
        FCUNIT_ASSERT(14 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 9b
    // realloc it with new size = 11b
    // note: left used <= right used, left used > increased sz
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 2, FMBUF_SEEK_RIGHT);
        FCUNIT_ASSERT(fmbuf_used(mbuf) == 9);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 11);
        FCUNIT_ASSERT(9 == fmbuf_used(new_buf));
        FCUNIT_ASSERT(2 == fmbuf_free(new_buf));
        FCUNIT_ASSERT(11 == fmbuf_size(new_buf));
        FCUNIT_ASSERT(5 == fmbuf_head_free(new_buf));
        FCUNIT_ASSERT(9 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 9b
    // realloc it with new size = 11b
    // note: left used > right used, left used > increased sz
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 8, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 6, FMBUF_SEEK_RIGHT);
        FCUNIT_ASSERT(fmbuf_used(mbuf) == 9);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 11);
        FCUNIT_ASSERT(9 == fmbuf_used(new_buf));
        FCUNIT_ASSERT(2 == fmbuf_free(new_buf));
        FCUNIT_ASSERT(11 == fmbuf_size(new_buf));
        FCUNIT_ASSERT(9 == fmbuf_head_free(new_buf));
        FCUNIT_ASSERT(5 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 2b
    // realloc it with new size = 5b
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 10, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 1, FMBUF_SEEK_RIGHT);
        FCUNIT_ASSERT(fmbuf_used(mbuf) == 2);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 5);
        FCUNIT_ASSERT(2 == fmbuf_used(new_buf));
        FCUNIT_ASSERT(3 == fmbuf_free(new_buf));
        FCUNIT_ASSERT(5 == fmbuf_size(new_buf));
        FCUNIT_ASSERT(5 == fmbuf_head_free(new_buf));
        FCUNIT_ASSERT(4 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }

    // mbuf: size = 10b, used = 2b
    // realloc it with new size = 1b
    {
        fmbuf* mbuf = fmbuf_create(10);
        fmbuf_head_seek(mbuf, 10, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(mbuf, 1, FMBUF_SEEK_RIGHT);
        FCUNIT_ASSERT(fmbuf_used(mbuf) == 2);

        fmbuf* new_buf = fmbuf_realloc(mbuf, 1);
        FCUNIT_ASSERT(2 == fmbuf_used(new_buf));
        FCUNIT_ASSERT(0 == fmbuf_free(new_buf));
        FCUNIT_ASSERT(2 == fmbuf_size(new_buf));
        FCUNIT_ASSERT(2 == fmbuf_head_free(new_buf));
        FCUNIT_ASSERT(1 == fmbuf_tail_free(new_buf));
        fmbuf_delete(new_buf);
    }
}

// test fmbuf push and pop
void test_mbuf1()
{
    // create a mbuf with size == 0
    {
        fmbuf* pbuf = fmbuf_create(0);
        FCUNIT_ASSERT(pbuf != NULL);
        FCUNIT_ASSERT(0 == fmbuf_size(pbuf));
        FCUNIT_ASSERT(0 == fmbuf_used(pbuf));
        FCUNIT_ASSERT(0 == fmbuf_free(pbuf));
        fmbuf_delete(pbuf);
    }

    fmbuf* pbuf = fmbuf_create(10);
    FCUNIT_ASSERT(pbuf!=NULL);

    char* push_buf[20];
    char* pop_buf[20];

    // push 1 byte data
    {
        int ret = fmbuf_push(pbuf, push_buf, 1);
        FCUNIT_ASSERT(0 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(1 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(9 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(9 == total_free);
    }

    // push 9 byte data
    {
        int ret = fmbuf_push(pbuf, push_buf, 9);
        FCUNIT_ASSERT(0 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(10 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(0 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(0 == total_free);
    }

    // continue push 1 byte data when mbuf is full
    {
        int ret = fmbuf_push(pbuf, push_buf, 1);
        FCUNIT_ASSERT(1 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(10 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(0 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(0 == total_free);
    }

    // clear and seek mbuf and push 5 bytes
    {
        fmbuf_clear(pbuf);
        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(10 == total_free);

        fmbuf_head_seek(pbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 6, FMBUF_SEEK_RIGHT);

        int ret = fmbuf_push(pbuf, push_buf, 5);
        FCUNIT_ASSERT(0 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(7 == used);

        total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(3 == total_free);
    }

    // push 4 bytes but the mbuf only have 3 bytes left
    {
        int ret = fmbuf_push(pbuf, push_buf, 4);
        FCUNIT_ASSERT(1 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(7 == used);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(3 == total_free);
    }

    // push 3 bytes the mbuf is full
    {
        int ret = fmbuf_push(pbuf, push_buf, 3);
        FCUNIT_ASSERT(0 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(10 == used);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(0 == total_free);

        ret = fmbuf_push(pbuf, push_buf, 1);
        FCUNIT_ASSERT(1 == ret);
    }

    //---------------mbuf pop-------------------
    //pop 1 bytes
    {
        fmbuf_clear(pbuf);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        int ret = fmbuf_pop(pbuf, pop_buf, 1);
        FCUNIT_ASSERT(1 == ret);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(0 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(10 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(10 == total_free);
    }

    // push 5 bytes and pop 1 bytes
    {
        int ret = fmbuf_push(pbuf, push_buf, 5);
        FCUNIT_ASSERT(0 == ret);

        ret = fmbuf_pop(pbuf, pop_buf, 1);
        FCUNIT_ASSERT(0 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(4 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(1 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(5 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(6 == total_free);
    }

    // pop 3 bytes and left 1 byte
    {
        int ret = fmbuf_pop(pbuf, pop_buf, 3);
        FCUNIT_ASSERT(0 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(1 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(4 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(5 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(9 == total_free);
    }

    // pop 2 bytes
    {
        int ret = fmbuf_pop(pbuf, pop_buf, 2);
        FCUNIT_ASSERT(1 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(1 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(4 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(5 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(9 == total_free);
    }

    // clear mbuf and seek tail < head
    {
        fmbuf_clear(pbuf);
        fmbuf_head_seek(pbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 2, FMBUF_SEEK_RIGHT);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(9 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(4 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(8 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(1 == total_free);
    }

    // total 9 bytes, pop 7 bytes
    {
        int ret = fmbuf_pop(pbuf, pop_buf, 7);
        FCUNIT_ASSERT(0 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(2 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(0 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(8 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(8 == total_free);
    }

    // pop 2 bytes and then the mbuf is empty
    {
        int ret = fmbuf_pop(pbuf, pop_buf, 2);
        FCUNIT_ASSERT(0 == ret);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(0 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(2 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(8 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(10 == total_free);
    }

    {
        fmbuf_clear(pbuf);
        fmbuf_head_seek(pbuf, 4, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 6, FMBUF_SEEK_RIGHT);

        int ret = fmbuf_pop(pbuf, NULL, 4);
        FCUNIT_ASSERT(ret == 1);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(2 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(4 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(4 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(8 == total_free);
    }

    // move 2 bytes
    {
        fmbuf_pop(pbuf, NULL, 2);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(0 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(6 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(4 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(10 == total_free);
    }

    // tail < head, tail_use == 5, but pop 6 bytes
    {
        fmbuf_clear(pbuf);
        fmbuf_head_seek(pbuf, 6, FMBUF_SEEK_RIGHT);
        fmbuf_tail_seek(pbuf, 4, FMBUF_SEEK_RIGHT);

        size_t used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(9 == used);

        fmbuf_pop(pbuf, NULL, 6);

        size_t size = fmbuf_size(pbuf);
        FCUNIT_ASSERT(10 == size);

        used = fmbuf_used(pbuf);
        FCUNIT_ASSERT(3 == used);

        size_t head_free = fmbuf_head_free(pbuf);
        FCUNIT_ASSERT(1 == head_free);

        size_t tail_free = fmbuf_tail_free(pbuf);
        FCUNIT_ASSERT(6 == tail_free);

        size_t total_free = fmbuf_free(pbuf);
        FCUNIT_ASSERT(7 == total_free);
    }

    fmbuf_delete(pbuf);
}

int main(int argc, char** argv)
{
    FCUNIT_RUN(test_mbuf);
    FCUNIT_RUN(test_mbuf1);
    FCUNIT_RUN(test_mbuf2);

    return 0;
}
