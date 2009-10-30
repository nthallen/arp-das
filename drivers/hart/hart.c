#include <signal.h>
#include "nortlib.h"
#include "gpib232.h"

void hart_output( char *cmd ) {
  char buf[80];
  
  sprintf( buf, "wrt #%d 22\n%s\r", strlen(cmd), cmd );
  gpib232_command( -2, buf );
}

int hart_input( char *buf, int count ) {
  return gpib232_read_data( buf, count, 22 );
}

void hart_readwrite( char *cmd ) {
  char buf[80];
  int nb;

  hart_output( cmd );
  nb = hart_input( buf, 32 );
  printf("%s", buf );
  fflush(stdout);
}

static int done = 0;
void sigint_handler( int sig ) {
  done = 1;
}

void main( int argc, char **argv ) {
  int n_channels = 3;
  
  if (argc > 1 ) {
	n_channels = atoi( argv[1] );
	if (n_channels < 1 || n_channels > 11 )
	  nl_error( 3, "Invalid number of channels" );
  }
  nl_error( 0, "Starting: %d channels", n_channels );

  gpib232_init( "/dev/ser1" );

  /* Initialize the gpib232 */
  gpib232_command( -2, "tmo 5\r" );

  /* Initialize device(s) */
  /* Set number of channels to 3 and run */
  { char buf[80];
	sprintf( buf, "scn=%d & sact=r\n", n_channels );
	hart_output( buf );
  }

  signal( SIGINT, sigint_handler );

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
  nl_error( 0, "Hart finished" );
}
