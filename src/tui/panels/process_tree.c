#if HAVE_NCURSES
#include <ncurses.h>
void panel_process_tree_draw(WINDOW *w) { (void)w; }
#else
void panel_process_tree_draw(void *w) { (void)w; }
#endif