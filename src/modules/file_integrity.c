#include "modules/base.h"
#include "utils/file_scan.h"
#include "utils/hash.h"
#include "core/logger.h"
#include "core/db.h"
#include "core/event_bus.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <errno.h>

static pthread_t g_thread;
static volatile int g_run = 0;
static int g_interval = 3600;

static int compute_merkle_root(const char *path, char outhex[65])
{
    if (!path || !outhex) return -1;
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    unsigned char buf[4096];
    unsigned char **blocks = NULL;
    size_t bcount = 0;
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX ctx; SHA256_Init(&ctx); SHA256_Update(&ctx, buf, r); SHA256_Final(hash, &ctx);
        unsigned char *hcopy = malloc(SHA256_DIGEST_LENGTH);
        if (!hcopy) { fclose(f); return -1; }
        memcpy(hcopy, hash, SHA256_DIGEST_LENGTH);
        unsigned char **tmp = realloc(blocks, sizeof(*blocks) * (bcount + 1));
        if (!tmp) { free(hcopy); fclose(f); return -1; }
        blocks = tmp; blocks[bcount++] = hcopy;
    }
    fclose(f);
    if (bcount == 0) {
        /* empty file -> hash of zero-length */
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char *)"", 0, hash);
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) sprintf(&outhex[i * 2], "%02x", hash[i]);
        outhex[64] = '\0';
        return 0;
    }
    while (bcount > 1) {
        size_t pairs = (bcount + 1) / 2;
        unsigned char **next = calloc(pairs, sizeof(*next));
        if (!next) { for (size_t i = 0; i < bcount; ++i) free(blocks[i]); free(blocks); return -1; }
        for (size_t i = 0; i < pairs; ++i) {
            unsigned char concat[SHA256_DIGEST_LENGTH * 2];
            unsigned char hash[SHA256_DIGEST_LENGTH];
            unsigned char *a = blocks[i * 2];
            unsigned char *b = (i * 2 + 1 < bcount) ? blocks[i * 2 + 1] : blocks[i * 2];
            memcpy(concat, a, SHA256_DIGEST_LENGTH);
            memcpy(concat + SHA256_DIGEST_LENGTH, b, SHA256_DIGEST_LENGTH);
            SHA256(concat, sizeof(concat), hash);
            unsigned char *hcopy = malloc(SHA256_DIGEST_LENGTH);
            if (!hcopy) { for (size_t j = 0; j < bcount; ++j) free(blocks[j]); free(blocks); for (size_t j = 0; j < i; ++j) free(next[j]); free(next); return -1; }
            memcpy(hcopy, hash, SHA256_DIGEST_LENGTH);
            next[i] = hcopy;
        }
        for (size_t i = 0; i < bcount; ++i) free(blocks[i]);
        free(blocks);
        blocks = next;
        bcount = pairs;
    }
    unsigned char *root = blocks[0];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) sprintf(&outhex[i * 2], "%02x", root[i]);
    outhex[64] = '\0';
    free(root);
    free(blocks);
    return 0;
}

static void file_cb(const char *path, void *ctx)
{
    (void)ctx;
    char h[65];
    char merkle[65];
    if (sha256_file_hex(path, h) == 0) {
        if (compute_merkle_root(path, merkle) != 0) merkle[0] = '\0';
        struct stat st; if (stat(path, &st) != 0) { nulleye_log(NYE_LOG_WARN, "stat failed for %s", path); }
        db_update_file_entry(path, h, merkle, st.st_mtime, st.st_size, st.st_mode & 0777, st.st_uid, st.st_gid, "");
        nuleye_event_t ev = {0};
        ev.ts = (uint64_t)time(NULL);
        ev.module = NYE_MODULE_FILE;
        strncpy(ev.path, path, sizeof(ev.path) - 1);
        strncpy((char *)ev.payload, h, sizeof(ev.payload) - 1);
        ev.payload_len = strlen(h);
        int pubrc = event_bus_publish(&ev);
        if (pubrc < 0) nulleye_log(NYE_LOG_WARN, "event_bus_publish failed (file_integrity): %d", pubrc);
    }
}

static void *scan_thread(void *arg)
{
    (void)arg;
    const char *paths[] = {"/etc", "/usr/bin", "/var/www", NULL};
    while (g_run) {
        for (const char **p = paths; *p; ++p) file_scan_path(*p, file_cb, NULL);
        sleep(g_interval);
    }
    return NULL;
}

static int fi_init(void)
{
    return 0;
}
static void fi_fini(void) {}
static void fi_start(void)
{
    g_run = 1;
    if (pthread_create(&g_thread, NULL, scan_thread, NULL) != 0) {
        nulleye_log(NYE_LOG_ERR, "file_integrity thread start failed: %s", strerror(errno));
        g_run = 0;
    }
}
static void fi_stop(void)
{
    g_run = 0;
    pthread_join(g_thread, NULL);
}

nuleye_module_t file_integrity_module = {
    .name = "FileIntegrity",
    .init = fi_init,
    .fini = fi_fini,
    .process_event = NULL,
    .tui_draw = NULL,
    .start = fi_start,
    .stop = fi_stop,
};
