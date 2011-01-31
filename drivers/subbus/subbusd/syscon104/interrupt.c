
static void delete_card( card_def **cdp ) {
  card_def *cd;

  if ( cdp == 0 || *cdp == 0 )
    nl_error( 4, "Invalid pointer to delete_card" );
  cd = *cdp;
  if ( sbwra( cd->address, 0 ) == 0 )
    nl_error( 1, "No acknowledge unprogramming %s (0x%03X)",
      cd->cardID, cd->address );
  regions[ cd->reg_id ].def[ cd->bitno ] = NULL;
  regions[ cd->reg_id ].bits &= ~(1 << cd->bitno );
  *cdp = cd->next;
  free( cd );
}

### Look at this
void service_expint( void ) {
  int i, bitno, bit;
  unsigned short mask;

  for ( i = 0; i < MAX_REGIONS && regions[i].address != 0; i++ ) {
    if ( regions[i].bits != 0 ) {
      mask = sbb( regions[i].address ) & regions[i].bits;
      for ( bitno = 0, bit = 1; i < 7 && mask; bitno++, bit <<= 1 ) {
        if ( mask & bit ) {
          card_def *cd = regions[i].defs[bitno];
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
            }
          } else nl_error( 2, "Unexpected interrupt in service_expint()";
          mask &= ~bit;
        }
      }
    }
  }
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
    prog = ( (region & 6) << 2 ) | 0x20 | bitno;
    if ( sbwra( address, 0 )    == 0 ||
         sbwra( address, prog ) == 0 )
      nl_error( 1, "No acknowledge programming %s(0x%03X)",
        cardID, address );
    else nl_error( -2, "Card %s assigned bit %d", cd->cardID,
          cd->bitno );
  }
  service_expint();

  return EOK;
}

int int_detach(int rcvid, subbusd_req_t *req) {
  subbusd_req_data2 *ireq = &req->data.d2;
  unsigned short addr;
  int rv = expint_detach( rcvid, ireq->cardID, &addr );
  if (rv != 0) return rv;
  if ( sbwra( addr, 0 ) == 0 )
    nl_error( 1, "No acknowledge unprogramming %s (0x%03X)",
      ireq->cardID, addr );
  return EOK;
}
