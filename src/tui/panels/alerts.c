#if HAVE_NCURSES
#include <ncurses.h>
void panel_alerts_draw(WINDOW *w) { (void)w; }
#else
void panel_alerts_draw(void *w) { (void)w; }
#endif