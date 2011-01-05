#include "DCnc.h"
#include "nctable.h"
#include "nortlib.h"

nc_data_client::nc_data_client( int bufsize_in, int fast, int non_block )
    : data_client(bufsize_in, 1, 0 ) {
}

void nc_data_client::read() {
  data_client::read();
  nct_refresh();
}

void nc_data_client::operate() {
  tminitfunc();
  while ( !dc_quit ) {
    read();
  }
}
