#define _GNU_SOURCE
#include "core/metrics.h"
#include "core/logger.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

static int listen_fd = -1;
static pthread_t metrics_thread;
static volatile int metrics_running = 0;

static const char metrics_response[] =
"# HELP nulleye_events_total Total events processed by NullEye\n"
"# TYPE nulleye_events_total counter\n"
"nulleye_events_total 0\n";

static void *metrics_loop(void *arg)
{
    (void)arg;
    while (metrics_running) {
        struct sockaddr_in cli;
        socklen_t clilen = sizeof(cli);
        int fd = accept(listen_fd, (struct sockaddr *)&cli, &clilen);
        if (fd < 0) {
            if (errno == EINTR) continue;
            nulleye_log(NYE_LOG_WARN, "metrics accept failed: %s", strerror(errno));
            break;
        }
        char req[512];
        ssize_t r = recv(fd, req, sizeof(req) - 1, 0);
        if (r > 0) {
            req[r] = '\0';
            if (strstr(req, "GET /metrics ") == req || strstr(req, "GET /metrics HTTP/1.") == req) {
                dprintf(fd, "HTTP/1.1 200 OK\r\nContent-Type: text/plain; version=0.0.4; charset=utf-8\r\nContent-Length: %zu\r\n\r\n%s", sizeof(metrics_response) - 1, metrics_response);            } else if (strstr(req, "GET /health ") == req || strstr(req, "GET /health HTTP/1.") == req) {
                const char *health = "{\"status\":\"ok\"}\n";
                dprintf(fd, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", strlen(health), health);            } else {
                dprintf(fd, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
            }
        }
        close(fd);
    }
    return NULL;
}

int metrics_start(unsigned short port)
{
    if (metrics_running) return 0;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) return -1;
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(listen_fd);
        listen_fd = -1;
        return -1;
    }
    if (listen(listen_fd, 5) != 0) {
        close(listen_fd);
        listen_fd = -1;
        return -1;
    }
    metrics_running = 1;
    if (pthread_create(&metrics_thread, NULL, metrics_loop, NULL) != 0) {
        metrics_running = 0;
        close(listen_fd);
        listen_fd = -1;
        return -1;
    }
    nulleye_log(NYE_LOG_INFO, "metrics server started on port %u", port);
    return 0;
}

void metrics_stop(void)
{
    if (!metrics_running) return;
    metrics_running = 0;
    if (listen_fd >= 0) {
        shutdown(listen_fd, SHUT_RDWR);
        close(listen_fd);
        listen_fd = -1;
    }
    pthread_join(metrics_thread, NULL);
}
