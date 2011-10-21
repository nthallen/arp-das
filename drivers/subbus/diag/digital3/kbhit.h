/*
  Test to support kbhit() getch() functions.
  kbhit() returns true if there is a character ready
  getch() returns the character
*/
#ifndef KBHIT_H_INCLUDED
#define KBHIT_H_INCLUDED
#include "nortlib.h"

extern int getch(void);
extern int kbhit(void);
extern int esc_c1, esc_c2;
#define KEY_ESCAPE 0x100
#define KEY_PGUP 0x101
#define KEY_PGDN 0x102
#define KEY_LEFT 0x103
#define KEY_RIGHT 0x104
#define KEY_DOWN 0x105
#define KEY_UP 0x106
#define KEY_F1 0x107

#endif
