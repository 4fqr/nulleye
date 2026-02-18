#define _GNU_SOURCE
#include "utils/file_scan.h"
#include <ftw.h>
#include <string.h>
#include <stdlib.h>

static void *g_cb_ctx = NULL;
static void (*g_cb)(const char *, void *) = NULL;

static int nftw_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    (void)sb; (void)typeflag; (void)ftwbuf;
    if (g_cb) g_cb(fpath, g_cb_ctx);
    return 0;
}

int file_scan_path(const char *path, void (*callback)(const char *path, void *ctx), void *ctx)
{
    if (!path || !callback) return -1;
    g_cb = callback; g_cb_ctx = ctx;
    nftw(path, nftw_cb, 16, FTW_PHYS);
    g_cb = NULL; g_cb_ctx = NULL;
    return 0;
}
