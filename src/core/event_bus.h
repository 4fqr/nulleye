#ifndef NYE_EVENT_BUS_H
#define NYE_EVENT_BUS_H

#include <stdint.h>
#include <stddef.h>

#define NYE_EVENT_PAYLOAD 256

typedef enum nuleye_module_id { NYE_MODULE_CORE = 0, NYE_MODULE_EBPF = 1, NYE_MODULE_FILE = 2, NYE_MODULE_PROC = 3, NYE_MODULE_NET = 4, NYE_MODULE_USER = 5, NYE_MODULE_AI = 6 } nuleye_module_id_t;

typedef struct {
    uint64_t ts;
    uint32_t module;
    uint32_t type;
    uint32_t pid;
    uint32_t ppid;
    uint32_t uid;
    uint32_t gid;
    char comm[16];
    char path[256];
    uint8_t payload[NYE_EVENT_PAYLOAD];
    size_t payload_len;
} nuleye_event_t;

int event_bus_init(size_t capacity);
void event_bus_shutdown(void);
int event_bus_publish(const nuleye_event_t *ev);
int event_bus_consume(nuleye_event_t *out);
size_t event_bus_pending(void);

#endif
