#ifndef NYE_MODULE_BASE_H
#define NYE_MODULE_BASE_H

#include "core/event_bus.h"

typedef struct nuleye_module {
    const char *name;
    int (*init)(void);
    void (*fini)(void);
    void (*process_event)(const nuleye_event_t *ev);
    void (*tui_draw)(void *panel_ctx);
    void (*start)(void);
    void (*stop)(void);
} nuleye_module_t;

#endif
