/* expint.c Provides all the specific code to handle support for
   the expints
   
   Registration is done with Region, cardID and a bit number is 
   assigned. Need to be able to detach based on cardID, need to 
   be able to trigger based on region and bit

   When a proxy arrives, go through our list of regions.
   For each region, read the INTA and compare the result to the 
   bits we've defined. If any are active, search through the list 
   of programs that have registered on that bit and Trigger the 
   appropriate proxy or proxies.
*/
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/kernel.h>
#include <signal.h>
#include "nortlib.h"
#include "subbus_int.h"

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
  regions[ cd->reg_id ].def[ cd->bitno ] = NULL;
  regions[ cd->reg_id ].bits &= ~(1 << cd->bitno );
  *cdp = cd->next;
  free( cd );
}

static card_def **find_card( char *cardID, unsigned short address ) {
  card_def *cd, **cdp;

  for ( cdp = &carddefs, cd = carddefs;
        cd != 0;
        cdp = &cd->next, cd = cd->next ) {
    if ( strncmp( cd->cardID, cardID, cardID_MAX ) == 0
          || cd->address == address ) {
      return cdp;
    }
  }
  return NULL;
}

/**
  * @return On success, returns zero and sets *card to new card_def. errno codes on failure.
  */
int expint_attach( int rcvid, char *cardID, unsigned short address,
                   int region, struct sigevent *event, card_def **card ) {
  card_def **cdp, *cd;
  int bitno, bit, i;

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
      // I used to check if the former owner was still alive.
      // I will use the implicit close() operation to
      // delete interrupt.
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
  //* Note that 8 is an arbitrary limitation of Syscon104.
  //* Nonetheless, we are unlikely to hit it with any architecture.
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
  cd->event = *event;
  cd->owner = rcvid;

  regions[i].bits |= bit;
  regions[i].def[bitno] = cd;
  
  cd->next = carddefs;
  carddefs = cd;
  *card = cd;
  return EOK;
}

int expint_detach( int rcvid, char *cardID, unsigned short *addr, unsigned int *bn ) {
  card_def **cdp;
  
  cdp = find_card( cardID, 0 );
  if ( cdp == 0 ) {
    return ENOENT;
  } else {
    if ( (*cdp)->owner != rcvid ) {
      nl_error( 1, "Non-owner %d attempted detach for %s",
                who, cardID );
      return EPERM;
    } else {
      *addr = cdp->address;
      *bn = cdp->bitno;
      delete_card( cdp );
      rep->status = EOK;
    }
  }
}
