#if HAVE_NCURSES
#include <ncurses.h>
void panel_config_editor_draw(WINDOW *w) { (void)w; }
#else
void panel_config_editor_draw(void *w) { (void)w; }
#endif