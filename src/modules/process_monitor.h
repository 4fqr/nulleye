#ifndef NYE_PROCESS_MONITOR_H
#define NYE_PROCESS_MONITOR_H

#include "modules/base.h"

extern nuleye_module_t process_monitor_module;

int process_monitor_lookup_pid(unsigned int pid, char *out, size_t outlen);

#endif