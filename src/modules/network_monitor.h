#ifndef NYE_NETWORK_MONITOR_H
#define NYE_NETWORK_MONITOR_H

#include "modules/base.h"

extern nuleye_module_t network_monitor_module;

int network_monitor_resolve_peer(const char *addr, char *out, size_t outlen);

#endif