#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "subbusd_int.h"
#include "sc104.h"
#include "nl_assert.h"

int service_expint( message_context_t * ctp, int code,
		     unsigned flags, void * handle ) {
  int i, bitno, bit;
  unsigned short mask;

  for ( i = 0; i < MAX_REGIONS && regions[i].address != 0; i++ ) {
    if ( regions[i].bits != 0 ) {
      mask = sbrb( regions[i].address ) & regions[i].bits;
      for ( bitno = 0, bit = 1; i < 7 && mask; bitno++, bit <<= 1 ) {
        if ( mask & bit ) {
          card_def *cd = regions[i].def[bitno];
          if (cd != NULL) {
            int rv = MsgDeliverEvent( cd->owner, &cd->event );
            unsigned short addr;
            unsigned int bn;
            if ( rv == -1 ) {
              switch (errno) {
                case EBADF:
                case ESRCH:
                  nl_error( 1,
                    "Process attached to '%s' interrupt not found",
                    cd->cardID );
                  rv = expint_detach( cd->owner, cd->cardID, &addr, &bn );
                  nl_assert( rv == EOK );
                  nl_assert( bitno == bn );
                  break;
                default:
                  nl_error( 4, "Unexpected error %d from MsgDeliverEvent: %s",
                      errno, strerror(errno));
              }
            } else {
	      nl_error(-2, "Interrupt delivered" );
            }
          } else nl_error( 2, "Unexpected interrupt in service_expint()");
          mask &= ~bit;
        }
      }
    }
  }
  return 0;
}

int int_attach(int rcvid, subbusd_req_t *req) {
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

  /* Now program the card! */
  { unsigned short prog;
    prog = ( (ireq->region & 6) << 2 ) | 0x20 | cd->bitno;
    if ( sbwra( cd->address, 0 )    == 0 ||
         sbwra( cd->address, prog ) == 0 )
      nl_error( 1, "No acknowledge programming %s(0x%03X)",
        cd->cardID, cd->address );
    else nl_error( -2, "Card %s assigned bit %d", cd->cardID,
          cd->bitno );
  }
  service_expint(NULL,0,0,NULL);

  return EOK;
}

int int_detach(int rcvid, subbusd_req_t *req) {
  subbusd_req_data2 *ireq = &req->data.d2;
  unsigned short addr;
  unsigned int bn;
  int rv = expint_detach( rcvid, ireq->cardID, &addr, &bn );
  if (rv != 0) return rv;
  if ( sbwra( addr, 0 ) == 0 )
    nl_error( 1, "No acknowledge unprogramming %s (0x%03X)",
      ireq->cardID, addr );
  return EOK;
}
