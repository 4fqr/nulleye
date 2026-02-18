#define _GNU_SOURCE
#include "ebpf/loader.h"
#include "core/logger.h"
#include "core/event_bus.h"
#include "core/config.h"
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

#if defined(__has_include)
#  if __has_include(<bpf/libbpf.h>)
#    include <bpf/libbpf.h>
#    include <bpf/bpf.h>
#    define NYE_HAVE_LIBBPF 1
#  else
#    define NYE_HAVE_LIBBPF 0
#  endif
#else
#  define NYE_HAVE_LIBBPF 0
#endif

#if NYE_HAVE_LIBBPF

#define MAX_BPF_OBJECTS 64
static struct bpf_object *g_objs[MAX_BPF_OBJECTS];
static int g_obj_count = 0;
static struct bpf_link *g_links[256];
static int g_link_count = 0;
static struct ring_buffer *rb = NULL;
static pthread_t rb_thread;
static volatile int rb_running = 0;

struct bpf_event {
    __u64 ts;
    __u32 pid;
    __u32 ppid;
    __u32 uid;
    __u32 gid;
    char comm[16];
    int retval;
    char filename[128];
};

static int handle_rb_event(void *ctx, void *data, size_t len)
{
    (void)ctx; (void)len;
    const struct bpf_event *e = data;
    nuleye_event_t ev = {0};
    ev.ts = e->ts;
    ev.module = NYE_MODULE_EBPF;
    ev.pid = e->pid;
    ev.uid = e->uid;
    strncpy(ev.comm, e->comm, sizeof(ev.comm) - 1);
    strncpy(ev.path, e->filename, sizeof(ev.path) - 1);
    int rc = event_bus_publish(&ev);
    if (rc < 0) nulleye_log(NYE_LOG_WARN, "event_bus_publish failed (ebpf): %d", rc);
    return 0;
}

static void *rb_poll(void *arg)
{
    (void)arg;
    while (rb_running) {
        int ret = ring_buffer__poll(rb, 100);
        if (ret == -EINTR) continue;
    }
    return NULL;
}

#else

static struct ring_buffer *rb = NULL;
static pthread_t rb_thread;
static volatile int rb_running = 0;

static int handle_rb_event(void *ctx, void *data, size_t len)
{
    (void)ctx; (void)data; (void)len;
    return 0;
}

static void *rb_poll(void *arg)
{
    (void)arg;
    while (rb_running) sleep(1);
    return NULL;
}

#endif
#if NYE_HAVE_LIBBPF

int ebpf_loader_start(void)
{
    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);

    DIR *d = opendir("src/ebpf/bpf");
    if (!d) {
        nulleye_log(NYE_LOG_ERR, "unable to open bpf directory");
        return -1;
    }
    struct dirent *ent;
    g_obj_count = 0;
    g_link_count = 0;
    int found_map_fd = -1;

    while ((ent = readdir(d)) != NULL) {
        if (!strstr(ent->d_name, ".bpf.o")) continue;
        if (g_obj_count >= MAX_BPF_OBJECTS) break;
        char path[512];
        snprintf(path, sizeof(path), "src/ebpf/bpf/%s", ent->d_name);
        struct bpf_object *obj = bpf_object__open_file(path, NULL);
        if (!obj) {
            nulleye_log(NYE_LOG_WARN, "failed to open %s", path);
            continue;
        }
        if (bpf_object__load(obj)) {
            nulleye_log(NYE_LOG_WARN, "failed to load %s", path);
            bpf_object__close(obj);
            continue;
        }
        g_objs[g_obj_count++] = obj;
        struct bpf_program *prog;
        bpf_object__for_each_program(prog, obj) {
            struct bpf_link *ln = bpf_program__attach(prog);
            if (ln) {
                if (g_link_count < (int)(sizeof(g_links) / sizeof(g_links[0]))) g_links[g_link_count++] = ln;
            }
        }
        struct bpf_map *m = bpf_object__find_map_by_name(obj, "events");
        if (m && found_map_fd < 0) found_map_fd = bpf_map__fd(m);
    }
    closedir(d);

    if (found_map_fd < 0) {
        nulleye_log(NYE_LOG_ERR, "no events map found in any BPF object");
        for (int i = 0; i < g_link_count; ++i) if (g_links[i]) bpf_link__destroy(g_links[i]);
        g_link_count = 0;
        for (int i = 0; i < g_obj_count; ++i) if (g_objs[i]) bpf_object__close(g_objs[i]);
        g_obj_count = 0;
        return -ENOENT;
    }

    rb = ring_buffer__new(found_map_fd, handle_rb_event, NULL, NULL);
    if (!rb) {
        nulleye_log(NYE_LOG_ERR, "ring_buffer__new failed");
        for (int i = 0; i < g_link_count; ++i) if (g_links[i]) bpf_link__destroy(g_links[i]);
        g_link_count = 0;
        for (int i = 0; i < g_obj_count; ++i) if (g_objs[i]) bpf_object__close(g_objs[i]);
        g_obj_count = 0;
        return -ENOMEM;
    }

    rb_running = 1;
    if (pthread_create(&rb_thread, NULL, rb_poll, NULL) != 0) {
        rb_running = 0;
        ring_buffer__free(rb);
        rb = NULL;
        for (int i = 0; i < g_link_count; ++i) if (g_links[i]) bpf_link__destroy(g_links[i]);
        g_link_count = 0;
        for (int i = 0; i < g_obj_count; ++i) if (g_objs[i]) bpf_object__close(g_objs[i]);
        g_obj_count = 0;
        nulleye_log(NYE_LOG_ERR, "failed to start ringbuffer thread");
        return -1;
    }
    nulleye_log(NYE_LOG_INFO, "eBPF loader started with %d objects and %d links", g_obj_count, g_link_count);
    return 0;
}

void ebpf_loader_stop(void)
{
    rb_running = 0;
    if (rb) {
        pthread_kill(rb_thread, SIGINT);
        pthread_join(rb_thread, NULL);
        ring_buffer__free(rb);
        rb = NULL;
    }
    for (int i = 0; i < g_link_count; ++i) if (g_links[i]) bpf_link__destroy(g_links[i]);
    g_link_count = 0;
    for (int i = 0; i < g_obj_count; ++i) if (g_objs[i]) bpf_object__close(g_objs[i]);
    g_obj_count = 0;
    nulleye_log(NYE_LOG_INFO, "eBPF loader stopped");
}

#else

int ebpf_loader_start(void)
{
    nulleye_log(NYE_LOG_WARN, "libbpf not available; eBPF disabled at build-time");
    return -1;
}

void ebpf_loader_stop(void)
{
    /* no-op when libbpf unavailable */
}

#endif