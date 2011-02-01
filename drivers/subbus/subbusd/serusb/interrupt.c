#include <stdio.h>
#include <errno.h>
#include "serusb.h"
#include "nortlib.h"


int int_attach(int rcvid, subbusd_req_t *req, char *sreq) {
  card_def *cd;
  subbusd_req_data2 *ireq = &req->data.d2;
  int rv;
  
  if ( ireq->region != 0x40 ) {
    nl_error( 2, "Requested interrupt region incompatible with hardware." );
    return EADDRNOTAVAIL;
  }
  rv = expint_attach( rcvid, ireq->cardID, ireq->address, ireq->region,
          &ireq->event, &cd );
  if ( rv != 0 ) return rv;
  snprintf( sreq, SB_SERUSB_MAX_REQUEST, "i%d:%04X\n", cd->bitno, ireq->address );
  return EOK;
}

int int_detach(int rcvid, subbusd_req_t *req, char *sreq) {
  subbusd_req_data2 *ireq = &req->data.d2;
  unsigned short addr;
  unsigned int bn;

  int rv = expint_detach( rcvid, ireq->cardID, &addr, &bn );
  if (rv != 0) return rv;
  snprintf(sreq, SB_SERUSB_MAX_REQUEST, "u%X\n", addr);
  return EOK;
}
