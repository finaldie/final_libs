#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ftimer/ftimer.h"
#include "fhash/fhash.h"

#define MAX_LINES 100000
static char* lines[MAX_LINES];
static int total_lines = 0;

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

    return memcmp(key1, key2, key_sz1);
}

static
void test()
{
    fhash_opt opt;
    opt.hash_alg = NULL;
    opt.compare = hash_core_compare;

    fhash* phash = fhash_create(0, opt, NULL, FHASH_MASK_NONE);

    {
        unsigned long long start = fgettime();
        for (int i = 0; i < total_lines; i++) {
            fhash_set(phash, lines[i], strlen(lines[i]),
                      lines[i], strlen(lines[i]));
        }
        unsigned long long end = fgettime();
        printf("fhash_set x%d spend time: %llu usec\n",
               total_lines, end - start);
    }

    {
        unsigned long long start = fgettime();
        for (int i = 0; i < total_lines; i++) {
            fhash_get(phash, lines[i], strlen(lines[i]), NULL);
        }
        unsigned long long end = fgettime();
        printf("fhash_get x%d spend time: %llu usec\n",
               total_lines, end - start);
    }

    {
        unsigned long long start = fgettime();
        for (int i = 0; i < total_lines; i++) {
            fhash_del(phash, lines[i], strlen(lines[i]));
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
    test();
    unload_key_values(lines, total_lines);
    return 0;
}
