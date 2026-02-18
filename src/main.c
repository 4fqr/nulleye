#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "core/event_bus.h"
#include "core/logger.h"
#include "core/config.h"
#include "core/db.h"
#include "core/module.h"
#include "ebpf/loader.h"
#include "core/metrics.h"
#include "core/response_engine.h"
#include "utils/sandbox.h"
#include "modules/ai_engine.h"
#include "modules/file_integrity.h"
#include "modules/process_monitor.h"
#include "modules/network_monitor.h"
#include "modules/user_monitor.h"
#include "tui/tui.h"

static volatile sig_atomic_t g_running = 1;

static void handle_signal(int signum)
{
    (void)signum;
    g_running = 0;
}

int main(int argc, char **argv)
{
    const char *config_path = "/etc/nulleye/config.yaml";
    int run_tui = 1;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--daemon") == 0) {
            run_tui = 0;
        } else if ((strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) && i + 1 < argc) {
            config_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("NullEye - usage: %s [-d|--daemon] [-c|--config /path/to/config.yaml]\n", argv[0]);
            return 0;
        }
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (logger_init(NULL) != 0) {
        fprintf(stderr, "logger_init failed\n");
        return 1;
    }

    nulleye_log(NYE_LOG_INFO, "Starting NullEye");

    if (config_load(config_path) != 0) {
        nulleye_log(NYE_LOG_ERR, "Failed to load config '%s'", config_path);
        return 1;
    }

    if (db_init(config_get_database()) != 0) {
        nulleye_log(NYE_LOG_ERR, "Database init failed");
        return 1;
    }

    if (event_bus_init(8192) != 0) {
        nulleye_log(NYE_LOG_ERR, "Event bus init failed");
        return 1;
    }

    module_register_builtin(&ai_module);
    module_register_builtin(&file_integrity_module);
    module_register_builtin(&process_monitor_module);
    module_register_builtin(&network_monitor_module);
    module_register_builtin(&user_monitor_module);

    if (module_load_all() != 0) {
        nulleye_log(NYE_LOG_WARN, "One or more modules failed to initialize");
    }

    if (ebpf_loader_start() != 0) {
        nulleye_log(NYE_LOG_WARN, "eBPF loader failed to start or disabled — continuing without kernel telemetry");
    }

#ifdef NYE_HAVE_LIBBPF
    nulleye_log(NYE_LOG_INFO, "eBPF support: compiled-in");
#else
    nulleye_log(NYE_LOG_INFO, "eBPF support: disabled at build-time");
#endif

#ifdef NYE_NO_LIBYAML
    nulleye_log(NYE_LOG_INFO, "YAML config: disabled (libyaml missing)");
#else
    nulleye_log(NYE_LOG_INFO, "YAML config: enabled");
#endif

#if HAVE_NCURSES
    nulleye_log(NYE_LOG_INFO, "TUI: ncurses enabled");
#else
    nulleye_log(NYE_LOG_INFO, "TUI: ncurses disabled — using stdout fallback");
#endif

    if (metrics_start(9100) != 0) nulleye_log(NYE_LOG_WARN, "metrics server failed to start");
    if (response_engine_init() != 0) nulleye_log(NYE_LOG_WARN, "response engine init failed");

    module_start_workers();

    if (sandbox_apply_limits() != 0) nulleye_log(NYE_LOG_WARN, "sandbox limits not fully applied");

    if (run_tui) {
        nulleye_log(NYE_LOG_INFO, "Launching TUI client");
        tui_run();
    } else {
        nulleye_log(NYE_LOG_INFO, "Running as daemon");
        while (g_running) {
            sleep(1);
        }
    }

    response_engine_fini();
    metrics_stop();

    nulleye_log(NYE_LOG_INFO, "Shutting down NullEye");

    module_stop_workers();
    ebpf_loader_stop();
    module_unload_all();
    event_bus_shutdown();
    db_close();
    logger_fini();
    return 0;
}
