#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "nortlib.h"
#include "oui.h"
#include "da_cache.h"
#include "gpib232.h"
#include "hartd.h"

/* hartd is a less-configurable version of hart to read
   a single temperature channel and report it to TM via
   da_cache. In addition to the standard DAS options,
   it will take a -a option to specify the cache address.
   The value it will report will be a double.
*/

static int done = 0;
unsigned short cache_addr = 0;

void hart_output( char *cmd ) {
  char buf[80];
  
  sprintf( buf, "wrt #%d 22\n%s\r", strlen(cmd), cmd );
  gpib232_command( -2, buf );
}

int hart_input( char *buf, int count ) {
  return gpib232_read_data( buf, count, 22 );
}

/* hart_readwrite() actually writes a command,
   then reads and finally writes the date to
   cache.
*/
void hart_readwrite( char *cmd ) {
  char buf[80];
  int nb;

  hart_output( cmd );
  nb = hart_input( buf, 32 );
  if ( strncmp( buf, "1: ", 3 ) == 0 ) {
    int rv;
	double val = strtod( buf+3, 0 );
	rv = cache_writev( cache_addr, sizeof(double), (void *)&val	);
	if ( rv == CACHE_E_NOCACHE ) {
	  nl_error( 0, "Cache has terminated: I will too" );
	  done = 1;
	} else if ( rv != CACHE_E_OK ) {
	  nl_error( 1, "Error %d from cache_writev", rv );
	}
  } else {
	nl_error( 1, "Invalid data from hart: '%s'", buf );
  }
}

void sigint_handler( int sig ) {
  done = 1;
}

void main( int argc, char **argv ) {
  int n_channels = 1;
  
  oui_init_options( argc, argv );
  if ( cache_addr == 0 )
	nl_error( 3, "Must specify a cache address" );
  nl_error( 0, "Starting: %d channel at address 0x%04X",
	n_channels, cache_addr );

  /* Initialize the gpib232 */
  gpib232_command( -2, "tmo 5\r" );

  /* Initialize device(s) */
  /* Set number of channels to n_channels and run */
  { char buf[80];
	sprintf( buf, "scn=%d & sact=r\n", n_channels );
	hart_output( buf );
  }

  signal( SIGINT, sigint_handler );
  signal( SIGHUP, sigint_handler );
  /* ### ask cmd_ctrl to send a signal on quit */

  while ( ! done ) {
	char buf[80];
	int nb;
	
	hart_output( "new\n" );
	nb = hart_input( buf, 10 );
	if ( nb <= 6 ) {
	  nl_error( 1, "Short read on new" );
	} else if ( buf[5] == '1' ) {
	  hart_readwrite( "tem\n" );
	} else if ( buf[5] != '0' ) {
	  nl_error( 1, "New returned %d bytes: '%s", nb, buf );
	}
	sleep(1);
  }

  /* Send shutdown commands to the devices */
  hart_output( "sact=st\n" );
  gpib232_command( -2, "loc 22\r" );

  /* Close our connections to Collection */

  /* Sign off */
  nl_error( 0, "Finished" );
}
