/* expint.c Provides all the specific code to handle support for
   the expints. This version is modified to run within idx64
   until such time as support for multiple subbus interrupt sources is required.
   
   Registration is done with Region, cardID and a bit number is 
   assigned. Need to be able to detach based on cardID, need to 
   be able to trigger based on region and bit

   A pulse is attached to the hardware interrupt.
   When the pulse arrives, we go through our list of regions.
   For each region, read the INTA and compare the result to the 
   bits we've defined. If any are active, search through the list 
   of programs that have registered on that bit and Trigger the
   appropriate action.
*/
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "intserv.h"
#include "intserv_int.h"
#include "idx64int.h"
#include "nortlib.h"
#include "subbus.h"

typedef struct carddef {
  struct carddef *next;
  char cardID[ cardID_MAX ];
  unsigned short address;
  unsigned int reg_id;
  unsigned int bitno;
  short pulse_code;
  int pulse_value;
} card_def;

typedef struct {
 unsigned short address;
 unsigned short bits;
 card_def *def[8];
} region;
#define MAX_REGIONS 4

card_def *carddefs;
region regions[ MAX_REGIONS ];

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

static card_def **find_card( const char *cardID, unsigned short address ) {
  card_def *cd, **cdp;

  for ( cdp = &carddefs, cd = carddefs;
        cd != 0;
        cdp = &cd->next, cd = cd->next ) {
    if ( strcmp( cd->cardID, cardID ) == 0
          || cd->address == address ) {
      return cdp;
    }
  }
  return NULL;
}

void service_expint( void ) {
  int i, bitno, bit;
  unsigned short mask;

  //while ( Creceive( expint_proxy, 0, 0 ) != -1 );
  for ( i = 0; i < MAX_REGIONS && regions[i].address != 0; i++ ) {
    if ( regions[i].bits != 0 ) {
      mask = sbb( regions[i].address ) & regions[i].bits;
      for ( bitno = 0, bit = 1; i < 7 && mask; bitno++, bit <<= 1 ) {
        if ( mask & bit ) {
          service_pulse( regions[i].def[bitno]->pulse_code,
            regions[i].def[bitno]->pulse_value );
          mask &= ~bit;
        }
      }
    }
  }
}

int expint_attach( /* pid_t who, */ const char *cardID,
      unsigned short address, int region,
      short pulse_code, int pulse_value ) {
  card_def **cdp, *cd;
  int bitno, bit, i;

  if ( subbus_subfunction == 0 ) {
    nl_error( 1, "Request for expint w/o subbus" );
    return ELIBACC;
  }
  /* Verify that the region is legal */
  if ( ( region & (~6) ) != 0x40 || address == 0 ) {
    nl_error( 1,
      "Illegal region (0x%02X) or address (0x%03X) requested",
      region, address );
    return ENXIO;
  }

  /* First check to make sure it isn't already defined */
  cdp = find_card( cardID, address );
  if ( cdp != 0 ) {
    cd = *cdp;
    if ( cd->address != address ) {
      nl_error( 2, "Same ID (%s) two addresses (0x%03X, 0x%03X)",
        cardID, cd->address, address );
      return ENXIO;
    } else {
      nl_error( 1, "Duplicate request for cardID %s", cardID );
      return EAGAIN;
    }
  }
  
  /* set up the region and select the bit */
  for ( i = 0; i < MAX_REGIONS && regions[i].address != 0; i++ ) {
    if ( regions[i].address == region ) break;
  }
  if ( i == MAX_REGIONS ) {
    nl_error( 2, "Too many regions requested!" );
    return ENOSPC;
  }
  regions[i].address = region;
  for ( bitno = 0, bit = 1; bitno < 8; bitno++, bit <<= 1 ) {
    if ( ( regions[i].bits & bit ) == 0 ) break;
  }
  if ( bitno == 8 ) {
    nl_error( 2, "Too many requests for region 0x%02X", region );
    return ENOSPC;
  }

  /* Now assign a new def. */
  cd = malloc( sizeof( card_def ) );
  if ( cd == 0 ) {
    nl_error( 2, "Out of memory in expint_attach" );
    return ENOMEM;
  }
  strncpy( cd->cardID, cardID, cardID_MAX );
  cd->cardID[ cardID_MAX - 1 ] = '\0';
  cd->address = address;
  cd->reg_id = i;
  cd->bitno = bitno;
  cd->pulse_code = pulse_code;
  cd->pulse_value = pulse_value;
  // cd->owner = who;

  regions[i].bits |= bit;
  regions[i].def[bitno] = cd;
  
  cd->next = carddefs;
  carddefs = cd;

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

int expint_detach( /* pid_t who, */ const char *cardID ) {
  card_def **cdp;
  
  cdp = find_card( cardID, 0 );
  if ( cdp == 0 ) {
    return ENOENT;
  } else {
    // if ( (*cdp)->owner != who ) {
      // nl_error( 1, "Non-owner %d attempted detach for %s",
                // who, cardID );
      // return EPERM;
    // } else {
      delete_card( cdp );
      return EOK;
    // }
  }
}

#ifdef IRQ_SUPPORT
  typedef struct irq_ctrl_s {
    struct irq_ctrl_s *next;
    char cardID[ cardID_MAX ];
    pid_t owner;
    pid_t proxy;
  } irq_ctrl;
  static struct {
    char *name;
    irq_ctrl *ctrl;
  } irq_defs[ ISRV_MAX_IRQS ] = {
    "SPARE_IRQ", 0,
    "PFAIL_IRQ", 0
  };

  void irq_attach( pid_t who, char *cardID, short irq,
                        pid_t proxy, IntSrv_reply *rep ) {
    irq_ctrl *ctrl;
    if ( irq < 0 || irq >= ISRV_MAX_IRQS ) {
      rep->status = ENOENT;
      return;
    }
    ctrl = new_memory(sizeof(irq_ctrl));
    ctrl->next = irq_defs[irq].ctrl;
    ctrl->owner = who;
    ctrl->proxy = proxy;
    strncpy( ctrl->cardID, cardID, cardID_MAX );
    ctrl->cardID[ cardID_MAX - 1 ] = '\0';
    irq_defs[irq].ctrl = ctrl;
    rep->status = EOK;
    if ( irq_defs[irq].ctrl->next == 0) {
      switch ( irq ) {
        case ISRV_IRQ_SPARE:
          spare_init();
          break;
        case ISRV_IRQ_PFAIL:
          pfail_init();
          break;
      }
    }
    /* should we check for the case where lowpower has already
       been asserted? */
    if ( irq == ISRV_IRQ_PFAIL ) pfail_proxy_handler( 0 );
  }

  void irq_detach( pid_t who, char *cardID, short irq, IntSrv_reply *rep ) {
    irq_ctrl **ctrlp, *ctrl;
    int found = 0;
    if ( irq < 0 || irq >= ISRV_MAX_IRQS ) {
      rep->status = ENOENT;
      return;
    }
    nl_error( 0, "irq detach request from %s", cardID );
    for ( ctrlp = &irq_defs[irq].ctrl; *ctrlp != 0; ctrlp = &(*ctrlp)->next )
      if ((*ctrlp)->owner == who) break;
    if ( *ctrlp != 0 ) {
      ctrl = *ctrlp;
      *ctrlp = ctrl->next;
      free_memory(ctrl);
      if ( irq_defs[irq].ctrl == 0 ) {
        nl_error( 0, "Detaching interrupt" );
        switch ( irq ) {
          case ISRV_IRQ_SPARE:
            spare_reset();
            break;
          case ISRV_IRQ_PFAIL:
            pfail_reset();
            break;
        }
        rep->status = EOK;
      }
    } else {
      nl_error( 2, "Failed attempt by %d to detach %s for %s",
        who, irq_defs[irq].name, cardID );
      rep->status = EPERM;
    }
  }

  void pfail_proxy_handler( int saw_irq ) {
    /* Check here to make sure it's legit */
    /* 30DH bit 0x10 is PFO/
            bit 0x04 is Lowpower */
    int iobits = inp(0x30D) ^ 0x10; /* port C hi byte */
    if ( iobits & 0x14) {
      irq_proxy_handler( ISRV_IRQ_PFAIL );
    } else if (saw_irq) {
      nl_error( 1, "Spurious lowpower interrupt observed" );
    }
  }

  void irq_proxy_handler( int irq ) {
    irq_ctrl *ctrl;
    if ( irq < 0 || irq >= ISRV_MAX_IRQS )
      nl_error( 4, "Bad irq number in irq_proxy_handler" );
    for (ctrl = irq_defs[irq].ctrl; ctrl; ctrl = ctrl->next ) {
      nl_error( 0, "preparing to send" );
      nl_error( 0, "Sending %d to %s", ctrl->proxy, ctrl->cardID );
      if ( ctrl->proxy < 0)
        kill(ctrl->owner, ~ctrl->proxy);
      else
        Trigger(ctrl->proxy);
    }
  }
#endif
