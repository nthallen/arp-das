/*
  Support kbhit() getch() functions.
  kbhit() returns true if there is a character ready
  getch() returns the character
*/
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/select.h>
#include <termios.h>
#include "nortlib.h"

static int term_isset = 0;
static struct termios tio_save;

static void reset_term(void) {
  tcsetattr(0, TCSANOW, &tio_save);
}

static void setup_term(void) {
  struct termios tio;
  if ( tcgetattr( 0, &tio ) == -1 )
    nl_error( 3, "Error %d from tcgetattr", errno);
  tio_save = tio;
  atexit(&reset_term);
  tio.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
  // tio.c_oflag &= ~OPOST;
  tio.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  tio.c_cflag &= ~(CSIZE|PARENB);
  tio.c_cflag |= CS8;
  tcsetattr( 0, TCSANOW, &tio );
  term_isset = 1;
}

int getch(void) {
  if (!term_isset) setup_term();
  return getchar();
}

int kbhit(void) {
  fd_set readfds;
  struct timeval tv;
  int rv;

  if (!term_isset) setup_term();
  FD_ZERO( &readfds );
  FD_SET( 0, &readfds );
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  rv = select( 1, &readfds, NULL, NULL, &tv );
  if ( rv == -1 )
    nl_error( 3, "Error %d from select", errno );
  return rv;
}
