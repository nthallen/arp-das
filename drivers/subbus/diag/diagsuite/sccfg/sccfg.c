/* sccfg to initialize attributes */
#include <ncurses/curses.h>
#include "cfg.h"
#include "attribut.h"

#define ESCAPE 27
int attributes[MAX_ATTRS];

#ifndef VERT_SINGLE
  #define VERT_SINGLE ACS_VLINE
#endif
#ifndef HORIZ_SINGLE
  #define HORIZ_SINGLE ACS_HLINE
#endif

#ifndef VERT_DOUBLE
  #define VERT_DOUBLE VERT_SINGLE
#endif
#ifndef HORIZ_DOUBLE
  #define HORIZ_DOUBLE HORIZ_SINGLE
#endif

#ifndef KEY_PGDN
  #define KEY_PGDN KEY_NPAGE
#endif
#ifndef KEY_PGUP
  #define KEY_PGUP KEY_PPAGE
#endif


void report_info(void) {
  char buf[80];
  snprintf( buf, 80, "COLORS is %d\n", COLORS );
  addstr(buf);
  snprintf( buf, 80, "COLOR_PAIRS is %d\n", COLOR_PAIRS );
  addstr(buf);
  snprintf( buf, 80, "%s color and %s change color\n",
    has_colors() ? "Has" : "Does not have",
    can_change_color() ? "can" : "cannot" );
  addstr(buf);
}

void get_h(void) {
  FILE *fp;
  int c;

  report_info();
  fp = fopen("attribut.h", "r");
  if (fp == NULL) return;
  for (;;) {
    c = getc(fp);
    if (c == EOF) break;
    addch(c);
  }
  fclose(fp);
}

void main(int argc, char **argv) {
  int i, c;
  short foreground, background;
  WINDOW *testw;

  initscr();
  start_color();
  init_attrs(CFG_FILE,attributes,MAX_ATTRS);
  clear();
  get_h();
  refresh();
  noecho();	       
  keypad(stdscr, TRUE);
  testw = newwin(10, 40, 7, 40);
  i = 0;
  pair_content(i+1, &foreground, &background);
  for (;;) {
    wattrset(testw, attributes[i]);
    wbkgdset(testw, attributes[i]);
    wclear(testw);
    box(testw, VERT_DOUBLE, HORIZ_DOUBLE);
    mvwprintw(testw, 5, 5, "Attribute %d, fg: %d bg: %d", i, foreground, background);
    mvwprintw(testw, 6, 7, "attr: %X", attributes[i] );
    wrefresh(testw);
    for (;;) {
      c = wgetch(stdscr);
      switch (c) {
        case KEY_PGDN:
          if (++i == MAX_ATTRS) i = 0;
          break;
        case KEY_PGUP:
          if (--i < 0) i = MAX_ATTRS-1;
          break;
        case KEY_LEFT:
          if (--foreground < 0) foreground = COLORS-1;
          if (foreground == background && --foreground < 0)
            foreground = COLORS-1;
          break;
        case KEY_RIGHT:
          if (++foreground == COLORS) foreground = 0;
          if (foreground == background && ++foreground == COLORS)
            foreground = 0;
          break;
        case KEY_UP:
          if ( --background < 0 ) background = COLORS-1;
          if ( foreground == background && --background < 0 )
            background = COLORS-1;
          break;
        case KEY_DOWN:
          if ( ++background == COLORS ) background = 0;
          if ( foreground == background && ++background == COLORS )
            background = 0;
          break;
        case 'b':
        case 'B':
          attributes[i] ^= A_BOLD;
          break;
        case 'd':
        case 'D':
          attributes[i] ^= A_DIM;
          break;
        case 'k':
        case 'K':
          attributes[i] ^= A_BLINK;
          break;
        case 'r':
        case 'R':
          attributes[i] ^= A_REVERSE;
          break;
        case 's':
        case 'S':
          attributes[i] ^= A_STANDOUT;
          break;
        case 'u':
        case 'U':
          attributes[i] ^= A_UNDERLINE;
          break;
        case ESCAPE:
          break;
        default:
          continue;
      }
      switch (c) {
        case KEY_PGUP:
        case KEY_PGDN:
          pair_content(i+1, &foreground, &background);
          break;
        case KEY_LEFT:
        case KEY_RIGHT:
        case KEY_UP:
        case KEY_DOWN:
          init_pair(i+1, foreground, background );
          break;
        default:
          break;
      }
      break;
    }
    if (c == ESCAPE) break;
  }
  refresh();
  delwin(testw);
  if (!save_attrs(CFG_FILE,attributes,MAX_ATTRS)) {
    mvaddstr(10,20,"Cannot open ");
    addstr(CFG_FILE);
    refresh();
    getch();
  }
  attrset(7);
  clear();
  refresh();
  endwin();
}
