/*
  Test to support kbhit() getch() functions.
  kbhit() returns true if there is a character ready
  getch() returns the character
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include "nortlib.h"
#include "kbhit.h"

static int term_isset = 0;
int esc_c1, esc_c2;
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

static void AlarmHandler( int signum ) { }

int getch(void) {
  int c;
  if (!term_isset) setup_term();
  c = getchar();
  if ( c == 0x1B ) {
    signal(SIGALRM, &AlarmHandler);
    alarm(1);
    esc_c1 = getchar();
    if ( esc_c1 == EOF ) {
      if ( errno != EINTR )
	nl_error( 4, "Unexpected error from getchar: %d", errno );
    } else {
      esc_c2 = getchar();
      if ( esc_c2 == EOF ) {
	if ( errno == EINTR ) {
	  ungetc(esc_c1, stdin);
	} else {
	  nl_error( 4, "Unexpected error from getchar: %d", errno );
	}
      }
      alarm(0);
      switch ( esc_c1 ) {
	case '[':
	  switch (esc_c2) {
	    case 'V': c = KEY_PGUP; break;
	    case 'U': c = KEY_PGDN; break;
	    case 'D': c = KEY_LEFT; break;
	    case 'C': c = KEY_RIGHT; break;
	    case 'B': c = KEY_DOWN; break;
	    case 'A': c = KEY_UP; break;

	    default:  c = KEY_ESCAPE; break;
	  }
	  break;
	case 'O':
	  switch (esc_c2) {
	    case 'P': c = KEY_F1; break;
	    default: c = KEY_ESCAPE; break;
	  }
	  break;
	default:
	  c = KEY_ESCAPE;
	  break;
      }
    }
    signal(SIGALRM, SIG_DFL);
  }
  return c;
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

