#ifndef NYE_FILE_SCAN_H
#define NYE_FILE_SCAN_H

int file_scan_path(const char *path, void (*callback)(const char *path, void *ctx), void *ctx);

#endif