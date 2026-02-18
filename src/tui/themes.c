#if HAVE_NCURSES
#include <ncurses.h>
void tui_apply_theme_dark(void) { init_pair(1, COLOR_CYAN, -1); }
#else
void tui_apply_theme_dark(void) { }
#endif