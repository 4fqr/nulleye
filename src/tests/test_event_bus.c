#include "../core/event_bus.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    assert(event_bus_init(4) == 0);
    nuleye_event_t ev = {0};
    ev.pid = 1234; ev.uid = 1000; ev.module = NYE_MODULE_CORE;
    /* fill capacity */
    for (int i = 0; i < 4; ++i) assert(event_bus_publish(&ev) == 0);
    /* next publish should return ENOBUFS */
    int rc = event_bus_publish(&ev);
    assert(rc < 0);
    nuleye_event_t out;
    int r = event_bus_consume(&out);
    assert(r == 1);
    assert(out.pid == 1234);
    event_bus_shutdown();
    printf("event_bus tests passed\n");
    return 0;
}
