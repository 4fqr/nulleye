#include "modules/base.h"
#include <stdio.h>

static int ex_init(void) { return 0; }
static void ex_fini(void) {}
static void ex_start(void) {}
static void ex_stop(void) {}

static nuleye_module_t plugin = {
    .name = "example_plugin",
    .init = ex_init,
    .fini = ex_fini,
    .process_event = NULL,
    .tui_draw = NULL,
    .start = ex_start,
    .stop = ex_stop,
};

nuleye_module_t *nulleye_module_get(void)
{
    return &plugin;
}
