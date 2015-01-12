#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flibs/ftime.h"
#include "flibs/fhash.h"

#define MAX_LINES 100000
static char* lines[MAX_LINES];
static int total_lines = 0;

static
void print_profile(fhash_profile_data* data)
{
    printf("[index]: used: %u, total: %u, usage rate: %f\n",
           data->index_used,
           data->index_size,
           data->index_size > 0
            ? (double)(data->index_used) / (double)(data->index_size)
            : 0.0);

    printf("[slots]: used: %zu, total: %zu, usage rate: %f\n",
           data->used_slots,
           data->total_slots,
           data->total_slots > 0
            ? (double)data->used_slots / (double)data->total_slots
            : 0.0);
}

static
int load_key_values(const char* file, char** line_bufs, int max_lines)
{
    FILE* fp = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(file, "r");
    if (!fp) {
        printf("cannot open %s\n", file);
        exit(1);
    }

    int i = 0;
    lines[i] = NULL;
    while ((read = getline(&line_bufs[i], &len, fp)) != -1 && i < max_lines) {
        line_bufs[++i] = NULL;
    }

    return i;
}

static
void unload_key_values(char** line_bufs, int max_lines)
{
    for (int i = 0; i < max_lines; i++) {
        free(line_bufs[i]);
    }
}

static
int hash_core_compare(const void* key1, key_sz_t key_sz1,
                      const void* key2, key_sz_t key_sz2)
{
    if (key_sz1 != key_sz2) {
        return 1;
    }

    return memcmp(key1, key2, (size_t)key_sz1);
}

static
void test_without_autorehash()
{
    printf("========= fhash testing without auto rehash =========\n");

    fhash_opt opt;
    opt.hash_alg = NULL;
    opt.compare = hash_core_compare;

    fhash* phash = fhash_create(0, opt, FHASH_MASK_NONE);

    // test set
    {
        unsigned long long start = fgettime();
        for (int i = 0; i < total_lines; i++) {
            fhash_set(phash, lines[i], (key_sz_t)strlen(lines[i]),
                      lines[i], (value_sz_t)strlen(lines[i]));
        }
        unsigned long long end = fgettime();
        printf("fhash_set x%d spend time: %llu usec\n",
               total_lines, end - start);
    }

    // test get
    {
        unsigned long long start = fgettime();
        for (int i = 0; i < total_lines; i++) {
            fhash_get(phash, lines[i], (key_sz_t)strlen(lines[i]), NULL);
        }
        unsigned long long end = fgettime();
        printf("fhash_get x%d spend time: %llu usec\n",
               total_lines, end - start);
    }

    // test iteration
    {
        unsigned long long start = fgettime();
        fhash_iter iter = fhash_iter_new(phash);
        char* data = NULL;
        int iter_count = 0;
        while ((data = fhash_next(&iter))) {
            iter_count++;
        }
        fhash_iter_release(&iter);
        unsigned long long end = fgettime();

        printf("fhash_next x%d spend time: %llu usec\n",
               iter_count, end -start);
    }

    // test rehash
    {
        unsigned long long start = fgettime();
        int ret = fhash_rehash(phash, (uint32_t)total_lines);
        unsigned long long end = fgettime();
        printf("fhash_rehash (index double), ret: %d, spend time: %llu usec\n",
               ret, end -start);
    }

    fhash_profile_data profile_data;
    fhash_profile(phash, 0, &profile_data);
    print_profile(&profile_data);

    // test delete
    {
        unsigned long long start = fgettime();
        for (int i = 0; i < total_lines; i++) {
            fhash_del(phash, lines[i], (key_sz_t)strlen(lines[i]));
        }
        unsigned long long end = fgettime();
        printf("fhash_del x%d spend time: %llu usec\n",
               total_lines, end - start);
    }

    fhash_delete(phash);
}

static
void test_with_autorehash()
{
    printf("========= fhash testing with auto rehash =========\n");

    fhash_opt opt;
    opt.hash_alg = NULL;
    opt.compare = hash_core_compare;

    fhash* phash = fhash_create(0, opt, FHASH_MASK_AUTO_REHASH);

    // test set
    {
        unsigned long long start = fgettime();
        for (int i = 0; i < total_lines; i++) {
            fhash_set(phash, lines[i], (key_sz_t)strlen(lines[i]),
                      lines[i], (value_sz_t)strlen(lines[i]));
        }
        unsigned long long end = fgettime();
        printf("fhash_set x%d spend time: %llu usec\n",
               total_lines, end - start);
    }

    // test get
    {
        unsigned long long start = fgettime();
        for (int i = 0; i < total_lines; i++) {
            fhash_get(phash, lines[i], (key_sz_t)strlen(lines[i]), NULL);
        }
        unsigned long long end = fgettime();
        printf("fhash_get x%d spend time: %llu usec\n",
               total_lines, end - start);
    }

    // test iteration
    {
        unsigned long long start = fgettime();
        fhash_iter iter = fhash_iter_new(phash);
        char* data = NULL;
        int iter_count = 0;
        while ((data = fhash_next(&iter))) {
            iter_count++;
        }
        fhash_iter_release(&iter);
        unsigned long long end = fgettime();

        printf("fhash_next x%d spend time: %llu usec\n",
               iter_count, end -start);
    }

    // test rehash
    {
        unsigned long long start = fgettime();
        int ret = fhash_rehash(phash, (uint32_t)total_lines);
        unsigned long long end = fgettime();
        printf("fhash_rehash (index double), ret: %d, spend time: %llu usec\n",
               ret, end -start);
    }

    fhash_profile_data profile_data;
    fhash_profile(phash, 0, &profile_data);
    print_profile(&profile_data);

    // test delete
    {
        unsigned long long start = fgettime();
        for (int i = 0; i < total_lines; i++) {
            fhash_del(phash, lines[i], (key_sz_t)strlen(lines[i]));
        }
        unsigned long long end = fgettime();
        printf("fhash_del x%d spend time: %llu usec\n",
               total_lines, end - start);
    }

    fhash_delete(phash);
}

int main(int argc, char** argv)
{
    total_lines = load_key_values("/tmp/fhash_kv.txt", lines, MAX_LINES);
    test_without_autorehash();
    test_with_autorehash();
    unload_key_values(lines, total_lines);
    return 0;
}
