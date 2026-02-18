#if HAVE_NCURSES
#include <ncurses.h>
void panel_graph_draw(WINDOW *w) { (void)w; }
#else
void panel_graph_draw(void *w) { (void)w; }
#endif