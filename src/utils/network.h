#ifndef NYE_NETWORK_H
#define NYE_NETWORK_H

#include <stddef.h>

int network_resolve_host(const char *ip, char *out, size_t outlen);

#endif