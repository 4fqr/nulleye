#if HAVE_NCURSES
#include <ncurses.h>
int tui_handle_key(int ch) { (void)ch; return 0; }
#else
int tui_handle_key(int ch) { (void)ch; return 0; }
#endif