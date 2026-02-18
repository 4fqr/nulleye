#if HAVE_NCURSES
#include <ncurses.h>
void dashboard_draw_overview(WINDOW *w) { (void)w; }
#else
void dashboard_draw_overview(void *w) { (void)w; }
#endif