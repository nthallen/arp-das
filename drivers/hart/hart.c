#include <signal.h>
#include "nortlib.h"
#include "gpib232.h"

static int done = 0;
void sigint_handler( int sig ) {
  done = 1;
}

static void hart_init(void) {
  char *buf[45];
  int nb;
  nb = gpib232_wrt_read( 22, "ver\n", buf, 25 );
  if (nb > 0 ) nl_error(0, "Hart: %s", buf);
  else nl_error(1, "Hart: ver request returned %d bytes", nb);
  // Now program the scanning method, units
  // This string specifies scanning 2 channels and reading Ohms
  gpib232_wrt( HART_ADDR, "scn=2 & scmo=1 & un=0 & sact=r\n" );
}

static void hart_shutdown(void) {
  char buf[50];
  
  /* Send shutdown commands to the devices */
  gpib232_wrt( HART_ADDR, "sact=st\n" );
  sprintf( buf, "loc %d\r\n", HART_ADDR );
  gpib232_cmd( buf );
}

void main( int argc, char **argv ) {
  int n_channels = 3;
  
  gpib232_init( "/net/radflt/dev/ser2" );
  hart_init();
  signal( SIGINT, sigint_handler );

  while ( ! done ) {
    char buf[80];
    int nb;
    
    nb = gpib232_wrt_read( HART_ADDR, "new\n", buf, 10 );
    if ( nb <= 6 ) {
      nl_error( 1, "Short read on new" );
    } else if ( buf[5] == '1' ) {
      nb = gpib232_wrt_read(HART_ADDR, "tem\n", buf, 20 );
      if ( nb > 0 ) nl_error( 0, "tem: %s", buf );
      else nl_error( 2, "tem returned %d", nb );
    } else if ( buf[5] != '0' ) {
      nl_error( 1, "New returned %d bytes: '%s", nb, buf );
    }
    sleep(1);
  }

  /* Send shutdown commands to the devices */
  hart_shutdown();

  /* Close our connections to Collection */

  /* Sign off */
  nl_error( 0, "Hart finished" );
}
