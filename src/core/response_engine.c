#define _GNU_SOURCE
#include "core/response_engine.h"
#include "core/logger.h"
#include "core/db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int response_engine_init(void)
{
    return 0;
}

void response_engine_fini(void)
{
}

int response_engine_perform(const char *action, const char *target, const char *reason)
{
    if (!action || !target) return -1;
    char buf[512];
    int n = snprintf(buf, sizeof(buf), "action=%s target=%s reason=%s", action, target, reason ? reason : "");
    if (n <= 0 || n >= (int)sizeof(buf)) return -1;
    nulleye_log(NYE_LOG_INFO, "response action: %s", buf);
    if (db_insert_alert("info", "response_engine", buf) != 0) {
        nulleye_log(NYE_LOG_WARN, "response_engine: failed to record action in DB");
        return -1;
    }
    return 0;
}
