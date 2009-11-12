#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include "nortlib.h"
#include "nl_assert.h"
#include "gpib232.h"

static int gpib_fd = -1;
static int gpib_tmo = 5;

#define OBUFSIZE 128

int gpib232_cmd( const char *cmd ) {
  int nb, nbw;
  nl_assert( cmd != NULL);
  nl_assert( gpib_fd != -1);
  nb = strlen(cmd);
  nbw = write(gpib_fd, cmd, nb);
  if ( nbw < nb ) {
    nl_error( 2, "Write to gpib_fd(%d) returned %d", nb, nbw );
  }
  return nbw;
}

// rep must be at least nb+1 bytes long
int gpib232_cmd_read( const char *cmd, char *rep, int nb ) {
  int nbr;
  int nbw = gpib232_cmd(cmd);
  if ( nbw <= 0 ) return 0;
  //sleep(1);
  nbr = readcond(gpib_fd, rep, nb, nb, 1, (gpib_tmo+1)*10 );
  if ( nbr >= 0 && nbr <= nb ) rep[nbr] = '\0';
  return nbr;
}

int gpib232_wrt( int addr, const char *cmd ) {
  char buf[OBUFSIZE];
  nl_assert(cmd != NULL);
  snprintf(buf, OBUFSIZE, "wrt #%d %d\n%s\r\n", strlen(cmd), addr, cmd );
  return gpib232_cmd( buf );
}

// rep must be long enough to hold nb+10 characters
int gpib232_wrt_read( int addr, const char *cmd, char *rep, int nb ) {
  char buf[OBUFSIZE];
  int nbr;

  nl_assert(cmd != NULL);
  nl_assert(rep != NULL);
  snprintf(buf, OBUFSIZE, "wrt #%d %d\n%s\r\nrd #%d %d\r\n",
            strlen(cmd), addr, cmd, nb, addr );
  nbr = gpib232_cmd_read( buf, rep, nb+10 );
  if (nbr < nb) {
    nl_error( 2, "Short read on command '%s': Requested %d, got %d",
      cmd, nb, nbr );
      return 0;
  } else {
    int i, nbrr = 0;
    for ( i = nb; i < nbr && isdigit(rep[i]); ++i ) {
      nbrr = nbrr*10 + rep[i] - '0';
    }
    if (nbrr > nb) {
      nl_error( 2, "Bad byte count in gpib232_wrt_read: requested %d, got %d",
        nb, nbrr );
      return 0;
    }
    rep[nbrr] = '\0';
    return nbrr;
  }
}

void gpib232_shutdown( void ) {
  /* This issues Untalk and Unlisten commands */
  if ( gpib_fd != -1 ) {
    gpib232_cmd( "cmd\n_?\r" ); // Untalk and Unlisten
    close( gpib_fd );
    gpib_fd = -1;
  }
}

#define IDBUFSIZE 256
/* gpib232_init(port) returns non-zero on error */
int gpib232_init( const char *port ) {
  char buf[IDBUFSIZE+10];
  int nb;

  gpib_fd = open( port, O_RDWR );
  if (gpib_fd == -1) {
    nl_error( 3, "Unable to open gpib232 port %s", port);
    return 1;
  }
  atexit( gpib232_shutdown );
  fcntl( gpib_fd, F_SETFL, O_NONBLOCK);
  for (;;) {
    nb = read( gpib_fd, buf, IDBUFSIZE );
    if ( nb <= 0 ) break;
    nl_error( 0, "GI: Cleared %d bytes", nb );
  }
  fcntl( gpib_fd, F_SETFL, 0);

  nb = gpib232_cmd_read( "id\r\n", buf, IDBUFSIZE );
  if ( nb == 0 ) nl_error( 3, "GPIB232 initialization ID failed" );
  else {
    int i;
    for ( i = 0; i < nb; ++i ) {
      if ( buf[i] != '\r' && buf[i] != '\n' ) {
	char *s = &buf[i];
	for (++i; i < nb && buf[i] != '\r' && buf[i] != '\n'; ++i);
	buf[i] = '\0';
	nl_error( 0, "GI: %s", s );
      }
    }
    // nl_error( 0, "GI(%d): %s", nb, buf );
  }
  sprintf( buf, "tmo %d\r\n", gpib_tmo );
  gpib232_cmd( buf );
  return 0;
}
