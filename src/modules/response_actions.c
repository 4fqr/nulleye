#include "modules/base.h"
#include "core/response_engine.h"
#include "core/logger.h"
#include <string.h>

int response_kill_process(unsigned int pid)
{
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "%u", pid);
    if (n <= 0) return -1;
    if (response_engine_perform("kill", buf, "policy") != 0) return -1;
    return 0;
}

int response_block_ip(const char *ip)
{
    if (!ip) return -1;
    if (response_engine_perform("block_ip", ip, "policy") != 0) return -1;
    return 0;
}
