/* getcon utility.
   Similar to the QNX4 utility of the same name, the
   purpose of getcon is to hold a console open until
   a display program is launched on it. This version
   will also report the console's ttyname in order to
   facilitate the subsequent launch.

   getcon <windowname> <pid>

   creates pty.<windowname>.<pid>.tmp
   invokes the ttyname() function and writes the result
   to the file.
   closes the file and renames it to pty.<windowname>.<pid>
   Listens on the "quit" channel for the signal to
   shutdown. Also listens for SIGHUP and SIGINT.
*/
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>
#include "nortlib.h"
#include "oui.h"
#include "nl_assert.h"
#include "tm.h"

static char *winname, *pid;
static int has_pid = 0;

void getcon_args( char *arg ) {
  nl_assert(arg != NULL);
  if ( winname == NULL ) {
    winname = arg;
  } else if ( pid == NULL ) {
    has_pid = 1;
    pid = arg;
  } else nl_error( 3, "Too many arguments" );
}

static void wait_for_quit(void) {
  #define CGBUFSIZE 80
  char buf[CGBUFSIZE+1];
  int fd = tm_open_name( tm_dev_name( "cmd/getcon" ), NULL, O_RDONLY );
  if ( fd == -1 )
    nl_error( 3, "Unable to open cmd/getcon: errno %d", errno );
  for (;;) {
    int nb = read(fd,buf,CGBUFSIZE);
    nl_assert( nb <= CGBUFSIZE );
    if (nb == -1)
      nl_error( 3, "Error reading from cmd/getcon: %d", errno );
    if ( nb == 0 ) break; // Canonical quit
    buf[nb] = '\0';
    if (buf[nb-1] == '\n') buf[nb-1] = '\0';
    if ( pid != NULL && strcmp(buf, pid) == 0 )
      break;
  }
}

int main( int argc, char **argv ) {
  char fname1[PATH_MAX], fname2[PATH_MAX];
  char *tty;
  FILE *fp;

  oui_init_options( argc, argv );
  if ( winname == NULL )
    nl_error( 3, "Must specify a window name" );
  if ( has_pid ) {
    snprintf( fname1, PATH_MAX, "pty.%s.%s.tmp", winname, pid );
    snprintf( fname2, PATH_MAX, "pty.%s.%s", winname, pid );
  } else {
    snprintf( fname1, PATH_MAX, "pty.%s.tmp", winname );
    snprintf( fname2, PATH_MAX, "pty.%s", winname );
  }
  tty = ttyname( STDOUT_FILENO );
  if (tty == NULL)
    nl_error( 3, "ttyname(1) returned error %d", errno );
  fp = fopen( fname1, "w" );
  if ( fp == NULL )
    nl_error( 3, "Unable to write to %s", fname1 );
  fprintf( fp, "%s\n", tty );
  fclose( fp );
  if ( rename( fname1, fname2 ) != 0 )
    nl_error( 3, "rename returned error %d", errno );
  wait_for_quit();
  return 0;
}

