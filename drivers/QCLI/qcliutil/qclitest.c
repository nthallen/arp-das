#include <stdlib.h>
#include "nortlib.h"
#include "qcliutil.h"
#include "oui.h"

#ifdef __USAGE
%C
  Runs diagnostics on QCLI V2
#endif

int main( int argc, char **argv ) {
  oui_init_options(argc, argv);
  if ( qcli_diags( 1 ) ) printf( "End of Tests\n" );
  else nl_error( 3, "Errors observed" );
  return 0;
}
