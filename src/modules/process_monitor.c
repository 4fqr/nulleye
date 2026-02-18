#include "modules/base.h"
#include "core/event_bus.h"
#include "core/logger.h"
#include "core/db.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static pthread_t g_thread;
static volatile int g_run = 0;

static void *proc_thread(void *arg)
{
    (void)arg;
    nuleye_event_t ev;
    while (g_run) {
        int r = event_bus_consume(&ev);
        if (r <= 0) { usleep(5000); continue; }
        if (ev.module == NYE_MODULE_EBPF) {
            db_insert_event(&ev, sizeof(ev), "process_monitor");
        }
    }
    return NULL;
}

static int pm_init(void) { return 0; }
static void pm_fini(void) {}
static void pm_start(void) { g_run = 1; if (pthread_create(&g_thread, NULL, proc_thread, NULL) != 0) { nulleye_log(NYE_LOG_ERR, "process_monitor thread start failed: %s", strerror(errno)); g_run = 0; } }
static void pm_stop(void) { g_run = 0; pthread_join(g_thread, NULL); }

nuleye_module_t process_monitor_module = {
    .name = "ProcessMonitor",
    .init = pm_init,
    .fini = pm_fini,
    .process_event = NULL,
    .tui_draw = NULL,
    .start = pm_start,
    .stop = pm_stop,
};
