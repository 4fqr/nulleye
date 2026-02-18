#include "modules/base.h"
#include "core/logger.h"
#include "core/db.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static pthread_t g_thread;
static volatile int g_run = 0;

static void *user_thread(void *arg)
{
    (void)arg;
    const char *path = "/var/log/auth.log";
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    char line[1024];
    while (g_run) {
        if (fgets(line, sizeof(line), f)) {
            db_insert_event(line, (int)strlen(line), "user_monitor");
        } else {
            sleep(1);
            clearerr(f);
        }
    }
    fclose(f);
    return NULL;
}

static int um_init(void) { return 0; }
static void um_fini(void) {}
static void um_start(void) { g_run = 1; if (pthread_create(&g_thread, NULL, user_thread, NULL) != 0) { nulleye_log(NYE_LOG_ERR, "user_monitor thread start failed: %s", strerror(errno)); g_run = 0; } }
static void um_stop(void) { g_run = 0; pthread_join(g_thread, NULL); }

nuleye_module_t user_monitor_module = {
    .name = "UserMonitor",
    .init = um_init,
    .fini = um_fini,
    .process_event = NULL,
    .tui_draw = NULL,
    .start = um_start,
    .stop = um_stop,
};
