#if HAVE_NCURSES
#include <ncurses.h>
void panel_network_map_draw(WINDOW *w) { (void)w; }
#else
void panel_network_map_draw(void *w) { (void)w; }
#endif