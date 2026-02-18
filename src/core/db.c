#define _GNU_SOURCE
#include "core/db.h"
#include "core/logger.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static sqlite3 *g_db = NULL;
static sqlite3_stmt *stmt_insert_event = NULL;
static sqlite3_stmt *stmt_insert_alert = NULL;
static sqlite3_stmt *stmt_store_model = NULL;
static sqlite3_stmt *stmt_load_model = NULL;
static sqlite3_stmt *stmt_upsert_file_entry = NULL;

static int exec_sql(const char *sql)
{
    char *err = NULL;
    int rc = sqlite3_exec(g_db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        nulleye_log(NYE_LOG_ERR, "SQL error: %s", err ? err : "unknown");
        sqlite3_free(err);
        return -1;
    }
    return 0;
}

int db_init(const char *path)
{
    if (sqlite3_open(path, &g_db) != SQLITE_OK) {
        nulleye_log(NYE_LOG_ERR, "sqlite3_open failed: %s", sqlite3_errmsg(g_db));
        return -1;
    }
    sqlite3_busy_timeout(g_db, 5000);
    exec_sql("PRAGMA journal_mode=WAL;");

    const char *schema =
        "CREATE TABLE IF NOT EXISTS events(id INTEGER PRIMARY KEY, ts INTEGER, module TEXT, data BLOB);"
        "CREATE TABLE IF NOT EXISTS file_entries(path TEXT PRIMARY KEY, hash TEXT, merkle TEXT, mtime INTEGER, size INTEGER, perms INTEGER, uid INTEGER, gid INTEGER, attrs TEXT);"
        "CREATE TABLE IF NOT EXISTS alerts(id INTEGER PRIMARY KEY, ts INTEGER, severity TEXT, module TEXT, message TEXT, acknowledged INTEGER DEFAULT 0);"
        "CREATE TABLE IF NOT EXISTS model(key TEXT PRIMARY KEY, blob BLOB);";
    if (exec_sql(schema) != 0) return -1;

    if (sqlite3_prepare_v2(g_db, "INSERT INTO events(ts,module,data) VALUES(strftime('%s','now'), ?, ?);", -1, &stmt_insert_event, NULL) != SQLITE_OK) { nulleye_log(NYE_LOG_ERR, "prepare stmt_insert_event failed: %s", sqlite3_errmsg(g_db)); return -1; }
    if (sqlite3_prepare_v2(g_db, "INSERT INTO alerts(ts,severity,module,message,acknowledged) VALUES(strftime('%s','now'), ?, ?, ?, 0);", -1, &stmt_insert_alert, NULL) != SQLITE_OK) { nulleye_log(NYE_LOG_ERR, "prepare stmt_insert_alert failed: %s", sqlite3_errmsg(g_db)); return -1; }
    if (sqlite3_prepare_v2(g_db, "REPLACE INTO model(key,blob) VALUES(?, ?);", -1, &stmt_store_model, NULL) != SQLITE_OK) { nulleye_log(NYE_LOG_ERR, "prepare stmt_store_model failed: %s", sqlite3_errmsg(g_db)); return -1; }
    if (sqlite3_prepare_v2(g_db, "SELECT blob FROM model WHERE key = ?;", -1, &stmt_load_model, NULL) != SQLITE_OK) { nulleye_log(NYE_LOG_ERR, "prepare stmt_load_model failed: %s", sqlite3_errmsg(g_db)); return -1; }
    if (sqlite3_prepare_v2(g_db, "REPLACE INTO file_entries(path,hash,merkle,mtime,size,perms,uid,gid,attrs) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?);", -1, &stmt_upsert_file_entry, NULL) != SQLITE_OK) {
        stmt_upsert_file_entry = NULL;
    }
    return 0;
}

void db_close(void)
{
    if (!g_db) return;
    sqlite3_finalize(stmt_insert_event);
    sqlite3_finalize(stmt_insert_alert);
    sqlite3_finalize(stmt_store_model);
    sqlite3_finalize(stmt_load_model);
    if (stmt_upsert_file_entry) sqlite3_finalize(stmt_upsert_file_entry);
    sqlite3_close(g_db);
    g_db = NULL;
}

int db_insert_event(const void *blob, int len, const char *module)
{
    if (!g_db) return -1;
    sqlite3_reset(stmt_insert_event);
    sqlite3_bind_text(stmt_insert_event, 1, module ? module : "unknown", -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt_insert_event, 2, blob, len, SQLITE_STATIC);
    int rc = sqlite3_step(stmt_insert_event);
    if (rc != SQLITE_DONE) {
        nulleye_log(NYE_LOG_ERR, "db_insert_event failed: %s", sqlite3_errmsg(g_db));
        return -1;
    }
    return 0;
}

int db_insert_alert(const char *severity, const char *module, const char *msg)
{
    if (!g_db) return -1;
    sqlite3_reset(stmt_insert_alert);
    sqlite3_bind_text(stmt_insert_alert, 1, severity, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert_alert, 2, module, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert_alert, 3, msg, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt_insert_alert);
    if (rc != SQLITE_DONE) {
        nulleye_log(NYE_LOG_ERR, "db_insert_alert failed: %s", sqlite3_errmsg(g_db));
        return -1;
    }
    return 0;
}

int db_store_model(const char *key, const void *blob, int len)
{
    if (!g_db) return -1;
    sqlite3_reset(stmt_store_model);
    sqlite3_bind_text(stmt_store_model, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt_store_model, 2, blob, len, SQLITE_STATIC);
    int rc = sqlite3_step(stmt_store_model);
    if (rc != SQLITE_DONE) { nulleye_log(NYE_LOG_ERR, "db_store_model failed: %s", sqlite3_errmsg(g_db)); return -1; }
    return 0;
}

int db_load_model(const char *key, void **blob, int *len)
{
    if (!g_db) return -1;
    sqlite3_reset(stmt_load_model);
    sqlite3_bind_text(stmt_load_model, 1, key, -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt_load_model);
    if (rc == SQLITE_ROW) {
        const void *data = sqlite3_column_blob(stmt_load_model, 0);
        int bytes = sqlite3_column_bytes(stmt_load_model, 0);
        *blob = malloc(bytes);
        if (!*blob) return -1;
        memcpy(*blob, data, bytes);
        *len = bytes;
        return 0;
    }
    if (rc != SQLITE_ROW && rc != SQLITE_DONE) nulleye_log(NYE_LOG_WARN, "db_load_model: no model for key %s", key ? key : "(null)");
    return -1;
}

int db_update_file_entry(const char *path, const char *hash, const char *merkle, long mtime, long size, int perms, int uid, int gid, const char *attrs)
{
    if (!g_db || !stmt_upsert_file_entry) return -1;
    sqlite3_reset(stmt_upsert_file_entry);
    sqlite3_bind_text(stmt_upsert_file_entry, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_upsert_file_entry, 2, hash ? hash : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_upsert_file_entry, 3, merkle ? merkle : "", -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt_upsert_file_entry, 4, mtime);
    sqlite3_bind_int64(stmt_upsert_file_entry, 5, size);
    sqlite3_bind_int(stmt_upsert_file_entry, 6, perms);
    sqlite3_bind_int(stmt_upsert_file_entry, 7, uid);
    sqlite3_bind_int(stmt_upsert_file_entry, 8, gid);
    sqlite3_bind_text(stmt_upsert_file_entry, 9, attrs ? attrs : "", -1, SQLITE_STATIC);
    return sqlite3_step(stmt_upsert_file_entry) == SQLITE_DONE ? 0 : -1;
}