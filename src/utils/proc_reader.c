#define _GNU_SOURCE
#include "utils/proc_reader.h"
#include <stdio.h>
#include <string.h>

int proc_read_cmdline(pid_t pid, char *out, size_t outlen)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    size_t r = fread(out, 1, outlen - 1, f);
    fclose(f);
    for (size_t i = 0; i < r; ++i) if (out[i] == '\0') out[i] = ' ';
    out[r] = '\0';
    return 0;
}
