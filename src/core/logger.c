#define _GNU_SOURCE
#include "core/logger.h"
#include "core/config.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>
#include <stdlib.h>

#define LOG_RING 1024
static char *g_ring[LOG_RING];
static size_t g_ring_idx = 0;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static FILE *g_logfile = NULL;

int logger_init(const char *path)
{
    const char *p = path ? path : config_get_log_file();
    g_logfile = fopen(p, "a");
    if (!g_logfile) return -1;
    setvbuf(g_logfile, NULL, _IOLBF, 0);
    openlog("nulleye", LOG_PID | LOG_CONS, LOG_DAEMON);
    return 0;
}

void logger_fini(void)
{
    for (size_t i = 0; i < LOG_RING; ++i) free(g_ring[i]);
    if (g_logfile) fclose(g_logfile);
    closelog();
}

static const char *level_str(nye_log_level_t lv)
{
    switch (lv) {
    case NYE_LOG_DEBUG: return "DEBUG";
    case NYE_LOG_INFO: return "INFO";
    case NYE_LOG_WARN: return "WARN";
    default: return "ERROR";
    }
}

void nulleye_log(nye_log_level_t level, const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    char out[1200];
    snprintf(out, sizeof(out), "%04d-%02d-%02d %02d:%02d:%02d [%s] %s\n",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec, level_str(level), buf);

    pthread_mutex_lock(&g_lock);
    if (g_logfile) fputs(out, g_logfile);
    syslog(level == NYE_LOG_ERR ? LOG_ERR : (level == NYE_LOG_WARN ? LOG_WARNING : LOG_INFO), "%s", buf);
    free(g_ring[g_ring_idx]);
    g_ring[g_ring_idx] = strdup(out);
    g_ring_idx = (g_ring_idx + 1) % LOG_RING;
    pthread_mutex_unlock(&g_lock);
}
