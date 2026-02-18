#define _GNU_SOURCE
#include "core/config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char g_database[256] = "/var/lib/nulleye/nulleye.db";
static char g_socket[108] = "/var/run/nulleye.sock";
static char g_logfile[256] = "/var/log/nulleye/nulleye.log";
static int g_ringbuf_size = 4096;

int config_load(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\0') continue;
        char key[128];
        char val[512];
        if (sscanf(p, "%127[^:]: %511[^\n]", key, val) == 2) {
            if (strcmp(key, "database") == 0) {
                strncpy(g_database, val, sizeof(g_database) - 1);
            } else if (strcmp(key, "socket") == 0) {
                strncpy(g_socket, val, sizeof(g_socket) - 1);
            } else if (strcmp(key, "log_file") == 0) {
                strncpy(g_logfile, val, sizeof(g_logfile) - 1);
            } else if (strcmp(key, "ringbuf_size") == 0 || strcmp(key, "ebpf_ringbuf_size") == 0) {
                g_ringbuf_size = atoi(val);
                if (g_ringbuf_size <= 0) g_ringbuf_size = 4096;
            }
        }
    }
    fclose(f);
    return 0;
}

const char *config_get_database(void)
{
    return g_database;
}

const char *config_get_socket_path(void)
{
    return g_socket;
}

const char *config_get_log_file(void)
{
    return g_logfile;
}

int config_get_ebpf_ringbuf_size(void)
{
    return g_ringbuf_size;
}
