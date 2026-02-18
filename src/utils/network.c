#include "utils/network.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

int network_resolve_host(const char *ip, char *out, size_t outlen)
{
    struct in_addr addr;
    if (inet_aton(ip, &addr) == 0) return -1;
    struct hostent *h = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (!h) return -1;
    strncpy(out, h->h_name, outlen - 1);
    out[outlen - 1] = '\0';
    return 0;
}
