/* sccfg to initialize attributes */
#include <curses.h>
#include <cfg.h>
#include "attribut.h"

#define ESCAPE 27
unsigned char attributes[MAX_ATTRS];

void get_h(void) {
  FILE *fp;
  int c;

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
  int i, c, foreground, background;
  WINDOW *testw;

  init_attrs(CFG_FILE,attributes,MAX_ATTRS);
  initscr();
  attrset(0x30);
  clear();
  get_h();
  refresh();
  noecho();	       
  keypad(stdscr, TRUE);
  testw = newwin(10, 40, 7, 40);
  i = 0;
  foreground = attributes[i] & 0xF;
  background = (attributes[i] >> 4) & 0xF;
  for (;;) {
    wattrset(testw, attributes[i]);
    wclear(testw);
    box(testw, VERT_DOUBLE, HORIZ_DOUBLE);
    mvwprintw(testw, 5, 10, "Attribute %d", i);
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
	  if (--foreground < 0) foreground = 0xF;
	  if (foreground == 0 && background == 0) foreground = 0xF;
	  break;
	case KEY_RIGHT:
	  if (++foreground == 0x10) foreground = 0;
	  if (foreground == 0 && background == 0) foreground = 1;
	  break;
	case KEY_UP:
	  background = (background-1) & 0xF;
	  break;
	case KEY_DOWN:
	  background = (background+1) & 0xF;
	  break;
        case ESCAPE:
	  break;
	default:
	  continue;
      }
      switch (c) {
        case KEY_PGUP:
	case KEY_PGDN:
	  foreground = attributes[i] & 0xF;
	  background = (attributes[i] >> 4) & 0xF;
	  break;
	case KEY_LEFT:
	case KEY_RIGHT:
	case KEY_UP:
	case KEY_DOWN:
	  attributes[i] = (background << 4) + foreground;
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
