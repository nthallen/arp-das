#include "ptrhm.h"

void ptrhm::check_coeffs() {
  int i, ok = 1, rv;
  unsigned short CR[7];
  char req[15];
  subbus_mread_req *creq;
  
  for (i = 0; i < 7; ++i) CR[i] = 0;
  snprintf(req, 15, "%X:2:%X", base_addr, base_addr + 12 );
  creq = pack_mread_request( 7, req );
  rv = mread_subbus( creq, CR );
  free_memory(creq);
  if ( rv > 0 ) {
    nl_error( 2, "PTRH[%s] Error %d reported by subbus_serusb", id, rv );
  } else {
    if ( rv < 0 )
      nl_error( 2, "PTRH[%s] No acknowledge reading coefficients", id );
    for (i = 1; i < 6; ++i) {
      if ( !( CR[6] & (2<<i)) )
        nl_error(2, "PTRH[%s] not responding for coefficient %d", id, i );
      if ( check_coeff(i, CR[i-1]) )
        ok = 0;
    }
    if (ok) nl_error(0, "PTRH[%s] Coefficients checked OK", id );
  }
  pack();
}

void ptrhm::pack() {
  char req[15];
  snprintf(req, 15, "%X:2:%X", base_addr + 12, base_addr + 26 );
  preq = pack_mread_request( 7, req );
}

void ptrhm::acquire() {
  if (preq == 0) pack();
  mread_subbus(preq, P);
}
