#if HAVE_NCURSES
#include <ncurses.h>
void panel_file_browser_draw(WINDOW *w) { (void)w; }
#else
void panel_file_browser_draw(void *w) { (void)w; }
#endif