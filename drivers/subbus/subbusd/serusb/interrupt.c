#include "serusb.h"


void service_expint( void ) {
  int i, bitno, bit;
  unsigned short mask;

  while ( Creceive( expint_proxy, 0, 0 ) != -1 );
  for ( i = 0; i < MAX_REGIONS && regions[i].address != 0; i++ ) {
    if ( regions[i].bits != 0 ) {
      mask = sbb( regions[i].address ) & regions[i].bits;
      for ( bitno = 0, bit = 1; i < 7 && mask; bitno++, bit <<= 1 ) {
        if ( mask & bit ) {
          if ( Trigger( regions[i].def[bitno]->proxy ) == -1 ) {
            if ( errno == ESRCH ) {
              card_def **cdp, *cd;

              cd = regions[i].def[bitno];
              nl_error( 1, "Proxy %d for card %s died",
                cd->proxy, cd->cardID );
              cdp = find_card( cd->cardID, cd->address );
              delete_card( cdp );
            } else
              nl_error( 2, "Unexpected error %d on Trigger", errno );
          }
          mask &= ~bit;
        }
      }
    }
  }
}

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
  snprintf( sreq, SB_SERUSB_MAX_REQ, "i%d:%04X\n", cd->bitno, ireq->address );
  return EOK;
}

int int_detach(int rcvid, subbusd_req_t *req, char *sreq) {
  subbusd_req_data2 *ireq = &req->data.d2;
  unsigned short addr;
  unsigned int bn;

  int rv = expint_detach( rcvid, ireq->cardID, &addr, &bn );
  if (rv != 0) return rv;
  snprintf(sreq, SB_SERUSB_MAX_REQ, "u%d\n", bn);
  return EOK;
}
