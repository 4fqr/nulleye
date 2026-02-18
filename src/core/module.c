#define _GNU_SOURCE
#include "core/module.h"
#include "core/logger.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>

#define PLUGINS_DIR "/usr/lib/nulleye/plugins"

static const nuleye_module_t **g_builtins = NULL;
static size_t g_bcount = 0;

int module_register_builtin(const nuleye_module_t *m)
{
    const nuleye_module_t **tmp = realloc(g_builtins, sizeof(*g_builtins) * (g_bcount + 1));
    if (!tmp) return -1;
    g_builtins = tmp;
    g_builtins[g_bcount++] = m;
    return 0;
}

int module_load_all(void)
{
    for (size_t i = 0; i < g_bcount; ++i) {
        const nuleye_module_t *m = g_builtins[i];
        if (m->init) {
            if (m->init() != 0) {
                nulleye_log(NYE_LOG_WARN, "module '%s' init failed", m->name);
            } else {
                nulleye_log(NYE_LOG_INFO, "module '%s' initialized", m->name);
            }
        }
    }

    DIR *d = opendir(PLUGINS_DIR);
    if (!d) return 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_type != DT_REG && ent->d_type != DT_LNK) continue;
        if (!strstr(ent->d_name, ".so")) continue;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", PLUGINS_DIR, ent->d_name);
        void *hdl = dlopen(path, RTLD_NOW);
        if (!hdl) {
            nulleye_log(NYE_LOG_WARN, "dlopen('%s') failed: %s", path, dlerror());
            continue;
        }
        nuleye_module_t *(*getmod)(void) = dlsym(hdl, "nulleye_module_get");
        if (!getmod) {
            nulleye_log(NYE_LOG_WARN, "plugin '%s' missing nulleye_module_get", path);
            dlclose(hdl);
            continue;
        }
        nuleye_module_t *pm = getmod();
        if (pm && pm->init) pm->init();
        nulleye_log(NYE_LOG_INFO, "loaded plugin %s", path);
    }
    closedir(d);
    return 0;
}

int module_unload_all(void)
{
    for (size_t i = 0; i < g_bcount; ++i) {
        const nuleye_module_t *m = g_builtins[i];
        if (m->fini) m->fini();
    }
    return 0;
}

void module_start_workers(void)
{
    for (size_t i = 0; i < g_bcount; ++i) if (g_builtins[i]->start) g_builtins[i]->start();
}

void module_stop_workers(void)
{
    for (size_t i = 0; i < g_bcount; ++i) if (g_builtins[i]->stop) g_builtins[i]->stop();
}
