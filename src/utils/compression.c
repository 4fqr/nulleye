#define _GNU_SOURCE
#include "utils/compression.h"
#include "core/logger.h"
#include <stdio.h>

int rollsum_blocks(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    unsigned char buf[4096];
    size_t r;
    size_t blocks = 0;
    while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
        (void)r;
        blocks++;
    }
    fclose(f);
    nulleye_log(NYE_LOG_DEBUG, "rollsum scanned %zu blocks for %s", blocks, path);
    return (int)blocks;
}
