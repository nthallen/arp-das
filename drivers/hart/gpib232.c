#include <stdlib.h>
#include "nortlib.h"
#include "gpib232.h"
char rcsid_gpib232_c[] =
  "$Header$";

FILE *gpib_ofp = NULL, *gpib_ifp = NULL;

static short int cta_read_int( void ) {
  char cbuf[80];
  int i;

  nl_error( -4, "Preparing to read an integer from CTA" );
  for (i = 0; i < 80; i++) {
	cbuf[i] = getc( gpib_ifp );
	if ( cbuf[i] == '\n' )
	  break;
  }
  cbuf[i+1] = '\0';
  nl_error( -4, "Integer read completed: %s", cbuf );
  return atoi( cbuf );
}

/* read data will read count bytes into the buffer from the specified GPIB
   address. It will then read the byte count and return that.
   If the byte count is equal to the requested count, it is possible
   that the device has more to say. The calling routine may wish
   to ask the GPIB-232CT-A for status information to determine if
   the transfer is complete. On the other hand, the calling routine
   may know that the transfer is incomplete, so calling for status
   may be unnecessary.
*/
int gpib232_read_data( char *buf, int count, int addr ) {
  int i;

  fprintf( gpib_ofp, "rd #%d %d\r", count, addr );
  fflush( gpib_ofp );
  nl_error( -4, "Preparing to read %d bytes from addr %d", count, addr );
  for (i = 0; i < count; i++) {
	buf[i] = getc( gpib_ifp );
  }
  nl_error( -4, "Read complete" );
  return cta_read_int();
}

/* Reads a double-precision value from GPIB address addr */
double gpib232_read_double( int addr ) {
  char buf[80];
  
  gpib232_read_data( buf, 15, addr );
  nl_error( -3, "Read for double: %s", buf );
  return atof( buf );
}

short int gpib232_read_status( void ) {
  short int gpib_stat;
  
  fputs( "stat n\r", gpib_ofp );
  fflush( gpib_ofp );
  gpib_stat = cta_read_int(); /* GPIB status */
  cta_read_int(); /* GPIB Error Conditions */
  cta_read_int(); /* Serial Port Error Conditions */
  cta_read_int(); /* Count */
  return gpib_stat;
}

void gpib232_command( int lvl, char *msg ) {
  lvl = lvl; /* later we can add logging! */
  if ( msg != 0 ) {
	fputs( msg, gpib_ofp );
	fflush( gpib_ofp );
  }
}

void gpib232_shutdown( void ) {
  /* This issues Untalk and Unlisten commands */
  if ( gpib_ofp != 0 ) {
	gpib232_command( -2, "cmd\n_?\r" );
	fclose( gpib_ofp );
	fclose( gpib_ifp );
	gpib_ofp = gpib_ifp = NULL;
  }
}

/* gpib232_init(port) returns non-zero on error */
int gpib232_init( const char *port ) {
  gpib_ofp = fopen( port, "w" );
  gpib_ifp = fopen( port, "r" );
  atexit( gpib232_shutdown );
  return (gpib_ofp == 0 || gpib_ifp == 0);
}
