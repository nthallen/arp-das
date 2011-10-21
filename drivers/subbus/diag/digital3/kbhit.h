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

#endif
