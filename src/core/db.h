#ifndef NYE_DB_H
#define NYE_DB_H

#include <stdint.h>

int db_init(const char *path);
void db_close(void);
int db_insert_event(const void *blob, int len, const char *module);
int db_insert_alert(const char *severity, const char *module, const char *msg);
int db_store_model(const char *key, const void *blob, int len);
int db_load_model(const char *key, void **blob, int *len);
int db_update_file_entry(const char *path, const char *hash, const char *merkle, long mtime, long size, int perms, int uid, int gid, const char *attrs);

#endif
