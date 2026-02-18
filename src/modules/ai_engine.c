#include "modules/base.h"
#include "core/event_bus.h"
#include "core/logger.h"
#include "ai/isolation_forest.h"
#include "ai/features.h"
#include "ai/model.h"
#include "core/db.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static iforest_t g_model;
static pthread_t g_thread;
static volatile int g_run = 0;
static int g_trees = 32;
static int g_threshold = 85;

static void *ai_thread(void *arg)
{
    (void)arg;
    nuleye_event_t ev;
    int window = 10000;
    int *feature_window = malloc(sizeof(int) * NYE_FEATURE_COUNT * window);
    int wpos = 0, wcount = 0;
    if (!feature_window) return NULL;
    while (g_run) {
        int r = event_bus_consume(&ev);
        if (r <= 0) { usleep(5000); continue; }
        int f[NYE_FEATURE_COUNT];
        ai_extract_features(&ev, f);
        memcpy(&feature_window[(wpos % window) * NYE_FEATURE_COUNT], f, sizeof(f));
        wpos = (wpos + 1) % window; if (wcount < window) wcount++;
        int score = iforest_score(&g_model, f, NYE_FEATURE_COUNT);
        if (score >= g_threshold) {
            db_insert_alert("high", "ai_engine", "anomalous event detected");
            nulleye_log(NYE_LOG_WARN, "AI anomaly score=%d for pid=%u comm=%s", score, ev.pid, ev.comm);
        }
        if (wcount == window) {
            iforest_free(&g_model);
            iforest_init(&g_model, g_trees, 8);
            ai_model_save("default", &g_model);
            wcount = 0;
        }
    }
    free(feature_window);
    return NULL;
}

static int ai_init(void)
{
    iforest_init(&g_model, g_trees, 8);
    ai_model_load("default", &g_model);
    return 0;
}
static void ai_fini(void)
{
    iforest_free(&g_model);
}
static void ai_start(void)
{
    g_run = 1;
    if (pthread_create(&g_thread, NULL, ai_thread, NULL) != 0) {
        nulleye_log(NYE_LOG_ERR, "ai thread start failed: %s", strerror(errno));
        g_run = 0;
    }
}
static void ai_stop(void)
{
    g_run = 0;
    pthread_join(g_thread, NULL);
}

nuleye_module_t ai_module = {
    .name = "AIEngine",
    .init = ai_init,
    .fini = ai_fini,
    .process_event = NULL,
    .tui_draw = NULL,
    .start = ai_start,
    .stop = ai_stop,
};
