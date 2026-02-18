#define _GNU_SOURCE
#include "core/event_bus.h"
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <errno.h>

static nuleye_event_t *g_ring = NULL;
static size_t g_capacity = 0;
static atomic_size_t g_head;
static atomic_size_t g_tail;

int event_bus_init(size_t capacity)
{
    if (capacity == 0) return -EINVAL;
    g_ring = calloc(capacity, sizeof(nuleye_event_t));
    if (!g_ring) return -ENOMEM;
    g_capacity = capacity;
    atomic_init(&g_head, 0);
    atomic_init(&g_tail, 0);
    return 0;
}

void event_bus_shutdown(void)
{
    free(g_ring);
    g_ring = NULL;
    g_capacity = 0;
}

int event_bus_publish(const nuleye_event_t *ev)
{
    if (!g_ring || !ev) return -EINVAL;
    size_t head = atomic_load_explicit(&g_head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&g_tail, memory_order_acquire);
    if (head - tail >= g_capacity) return -ENOBUFS;
    size_t idx = atomic_fetch_add_explicit(&g_head, 1, memory_order_acq_rel);
    size_t pos = idx % g_capacity;
    memcpy(&g_ring[pos], ev, sizeof(nuleye_event_t));
    return 0;
}

int event_bus_consume(nuleye_event_t *out)
{
    if (!g_ring || !out) return -1;
    size_t tail = atomic_fetch_add_explicit(&g_tail, 1, memory_order_acq_rel);
    size_t head = atomic_load_explicit(&g_head, memory_order_acquire);
    if (tail >= head) {
        atomic_fetch_sub_explicit(&g_tail, 1, memory_order_acq_rel);
        return 0;
    }
    size_t pos = tail % g_capacity;
    memcpy(out, &g_ring[pos], sizeof(nuleye_event_t));
    return 1;
}

size_t event_bus_pending(void)
{
    size_t head = atomic_load_explicit(&g_head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&g_tail, memory_order_acquire);
    return head - tail;
}
