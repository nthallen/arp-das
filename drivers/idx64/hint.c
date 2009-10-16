/* hint.c provides all the interrupt functionality (except the
   handler itself.
*/
#include <conio.h>
#include <i86.h>
#include <sys/irqinfo.h>
#include <sys/proxy.h>
#include "nortlib.h"
#include "intserv.h"
#include "internal.h"
#include "subbus.h"

static int expint_iid = -1;
static int spare_iid = -1;
static int pfail_iid = -1;
pid_t expint_proxy = 0;
pid_t spare_proxy = 0;
pid_t pfail_proxy = 0;
int expint_irq = 9;
int spare_irq = 0;
int pfail_irq = 0;

#define MAX_IRQ_104 12
static unsigned short irq104[ MAX_IRQ_104 ] = {
  0, 0, 0, 0x21, 0x22, 0x23, 0x24, 0x25, 0, 0x20, 0x26, 0x27
};

static int int_init( int irq, unsigned short enable, int bit,
          pid_t (far *handler)( void ) ) {
  unsigned short cfg_word, cfg_val, cfg_mask;
  int iid;

  if ( irq < 0 || irq >= MAX_IRQ_104 || irq104[ irq ] == 0 )
    nl_error( 3, "IRQ %d is invalid", irq );
  iid = qnx_hint_attach( irq, handler, FP_SEG( &expint_proxy ) );
  if (iid == -1)
    nl_error( 3, "Unable to attach IRQ %d", irq);
  if ( subbus_subfunction == SB_SYSCON104 ) {
    /* It is theoretically possible to run these cards on a
       cable off of an older syscon, so I won't die hard if
       this isn't syscon104.
    */
    cfg_val = irq104[ irq ] & ~0x20;
    cfg_val = ( cfg_val << bit ) | enable;
    cfg_mask = ( 7 << bit ) | enable;
    cfg_word = ( inpw( 0x312 ) & ~cfg_mask ) | cfg_val;
    nl_error( -2, "SC104 writing CPA word %04X", cfg_word );
    outpw( 0x312, cfg_word );
  }
  return iid;
}

static void int_reset( int iid, unsigned short mask ) {
  if ( iid != -1 ) qnx_hint_detach( iid );
  if ( subbus_subfunction == SB_SYSCON104 ) {
    unsigned short cfg_word = ( inpw( 0x312 ) & ~mask );
    outpw( 0x312, cfg_word );
  }
}

void expint_init(void) {
  /* Get a proxy for the handler */
  expint_proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
  if ( expint_proxy == -1 )
    nl_error( 3, "Unable to attach proxy for expint" );

  expint_iid = int_init( expint_irq, 0x20, 0, expint_handler );
}

void expint_reset(void) {
  int_reset( expint_iid, 0x3F );
}

void spare_init( void ) {
  spare_iid = int_init( spare_irq, 0x2000, 10, spare_handler );
}

void spare_reset( void ) {
  int_reset( spare_iid, 0x3C00 );
}

void pfail_init( void ) {
  if ( pfail_proxy == 0 ) {
    pfail_proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
	outp( 0x316, 0); /* reset the lowpower signal */
    if ( pfail_proxy == -1 )
      nl_error( 3, "Unable to attach proxy for pfail" );

    pfail_iid = int_init( pfail_irq, 0x200, 6, pfail_handler );
  }
}

void pfail_reset( void ) {
  int_reset( pfail_iid, 0x3C0 );
  set_failure(1);
  if ( qnx_proxy_detach( pfail_proxy ) ) {
    nl_error(1, "Error detaching pfail_proxy");
  }
  pfail_proxy = 0;
}
