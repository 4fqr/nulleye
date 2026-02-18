#include "modules/base.h"
#include "core/event_bus.h"
#include "core/logger.h"
#include "core/db.h"
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

static pthread_t g_thread;
static volatile int g_run = 0;

static void *net_thread(void *arg)
{
    (void)arg;
    nuleye_event_t ev;
    while (g_run) {
        int r = event_bus_consume(&ev);
        if (r <= 0) { usleep(5000); continue; }
        if (ev.module == NYE_MODULE_EBPF) {
            if (ev.path[0] == '\0') {
                db_insert_event(&ev, sizeof(ev), "network_monitor");
            }
        }
    }
    return NULL;
}

static int nm_init(void) { return 0; }
static void nm_fini(void) {}
static void nm_start(void) { g_run = 1; if (pthread_create(&g_thread, NULL, net_thread, NULL) != 0) { nulleye_log(NYE_LOG_ERR, "network_monitor thread start failed: %s", strerror(errno)); g_run = 0; } }
static void nm_stop(void) { g_run = 0; pthread_join(g_thread, NULL); }

nuleye_module_t network_monitor_module = {
    .name = "NetworkMonitor",
    .init = nm_init,
    .fini = nm_fini,
    .process_event = NULL,
    .tui_draw = NULL,
    .start = nm_start,
    .stop = nm_stop,
};
