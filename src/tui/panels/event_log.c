#if HAVE_NCURSES
#include <ncurses.h>
void panel_event_log_draw(WINDOW *w) { (void)w; }
#else
void panel_event_log_draw(void *w) { (void)w; }
#endif