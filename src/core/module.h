#ifndef NYE_MODULE_H
#define NYE_MODULE_H

#include "modules/base.h"

int module_register_builtin(const nuleye_module_t *m);
int module_load_all(void);
int module_unload_all(void);
void module_start_workers(void);
void module_stop_workers(void);

#endif