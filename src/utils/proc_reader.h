#ifndef NYE_PROC_READER_H
#define NYE_PROC_READER_H

#include <sys/types.h>
#include <stddef.h>

int proc_read_cmdline(pid_t pid, char *out, size_t outlen);

#endif