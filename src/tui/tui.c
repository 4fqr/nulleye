#define _GNU_SOURCE
#include "tui/tui.h"
#include "core/event_bus.h"
#include "core/logger.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define MAX_EVENTS 1024
static nuleye_event_t g_events[MAX_EVENTS];
static int g_ehead = 0;
static int g_erun = 0;
static pthread_t g_consumer;

static void *consumer(void *arg)
{
    (void)arg;
    while (g_erun) {
        nuleye_event_t ev;
        int r = event_bus_consume(&ev);
        if (r > 0) {
            g_events[g_ehead % MAX_EVENTS] = ev;
            g_ehead++;
        } else {
            usleep(2000);
        }
    }
    return NULL;
}

#if HAVE_NCURSES
#include <ncurses.h>
#include <panel.h>

static void draw_main(WINDOW *w)
{
    werase(w);
    box(w, 0, 0);
    mvwprintw(w, 0, 2, " NullEye — Events ");
    int rows, cols; getmaxyx(w, rows, cols);
    int start = g_ehead > rows - 3 ? g_ehead - (rows - 3) : 0;
    int row = 1;
    for (int i = start; i < g_ehead && row < rows - 1; ++i, ++row) {
        nuleye_event_t *ev = &g_events[i % MAX_EVENTS];
        mvwprintw(w, row, 1, "%5u %-16s %-30s", ev->pid, ev->comm, ev->path[0] ? ev->path : "-");
    }
    wnoutrefresh(w);
}

int tui_run(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_WHITE, -1);

    int rows, cols; getmaxyx(stdscr, rows, cols);
    WINDOW *mainw = newwin(rows - 2, cols - 2, 1, 1);

    g_erun = 1;
    if (pthread_create(&g_consumer, NULL, consumer, NULL) != 0) {
        nulleye_log(NYE_LOG_ERR, "tui consumer thread start failed: %s", strerror(errno));
        g_erun = 0;
    }

    while (1) {
        clear();
        mvprintw(0, 2, "NullEye — Press 'q' to quit");
        draw_main(mainw);
        doupdate();
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;
        usleep(80000);
    }

    g_erun = 0;
    pthread_join(g_consumer, NULL);
    delwin(mainw);
    endwin();
    return 0;
}

#else

static void draw_main_fallback(FILE *out)
{
    fprintf(out, "--- NullEye events (last 20) ---\n");
    int start = g_ehead > 20 ? g_ehead - 20 : 0;
    for (int i = start; i < g_ehead; ++i) {
        nuleye_event_t *ev = &g_events[i % MAX_EVENTS];
        fprintf(out, "%5u %-16s %s\n", ev->pid, ev->comm, ev->path[0] ? ev->path : "-");
    }
    fflush(out);
}

int tui_run(void)
{
    g_erun = 1;
    if (pthread_create(&g_consumer, NULL, consumer, NULL) != 0) {
        nulleye_log(NYE_LOG_ERR, "tui consumer thread start failed: %s", strerror(errno));
        g_erun = 0;
    }
    while (1) {
        printf("\033c");
        draw_main_fallback(stdout);
        printf("Press Ctrl-C to exit\n");
        sleep(1);
    }
    g_erun = 0;
    pthread_join(g_consumer, NULL);
    return 0;
}
#endif
