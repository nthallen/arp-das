/* hint.c provides all the interrupt functionality (except the
   handler itself.
*/
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include "nortlib.h"
#include "intserv.h"
#include "subbus.h"

static int expint_iid = -1;
int expint_irq = 9;

#define MAX_IRQ_104 12
static unsigned short irq104[ MAX_IRQ_104 ] = {
  0, 0, 0, 0x21, 0x22, 0x23, 0x24, 0x25, 0, 0x20, 0x26, 0x27
};

static int read_one_irq( char **s ) {
  int ans;
  char *t;

  t = *s;
  if ( t == 0 || *t == '\0' ) return 0;
  ans = strtoul( t, s, 10 );
  if ( t == *s ) ans = 0;
  if ( **s == ':' ) (*s)++;
  return ans;
}

void process_IRQs( char *t ) {
  expint_irq = read_one_irq( &t );
  //spare_irq = read_one_irq( &t );
  //pfail_irq = read_one_irq( &t );
}

static int int_init( int irq, unsigned short enable, int bit,
           int coid, short code, int value ) {
  unsigned short cfg_word, cfg_val, cfg_mask;
  int iid;
  struct sigevent intr_event;

  if ( irq < 0 || irq >= MAX_IRQ_104 || irq104[ irq ] == 0 )
    nl_error( 3, "IRQ %d is invalid", irq );
  intr_event.sigev_notify = SIGEV_PULSE;
  intr_event.sigev_coid = coid;
  intr_event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
  intr_event.sigev_code = code;
  intr_event.sigev_value.sival_int = value;
  iid = InterruptAttachEvent(expint_irq, &intr_event,
      _NTO_INTR_FLAGS_PROCESS | _NTO_INTR_FLAGS_TRK_MSK );
  if (iid == -1)
    nl_error( 3, "Unable to attach IRQ %d: errno %d", irq, errno);
  if ( subbus_subfunction == SB_SYSCON104 ) {
    /* It is theoretically possible to run these cards on a
       cable off of an older syscon, so I won't die hard if
       this isn't syscon104.
    */
    cfg_val = irq104[ irq ] & ~0x20;
    cfg_val = ( cfg_val << bit ) | enable;
    cfg_mask = ( 7 << bit ) | enable;
    cfg_word = ( in16( 0x312 ) & ~cfg_mask ) | cfg_val;
    nl_error( -2, "SC104 writing CPA word %04X", cfg_word );
    out16( 0x312, cfg_word );
  }
  return iid;
}

static void int_reset( int iid, unsigned short mask ) {
  if ( iid != -1 ) InterruptDetach( iid );
  if ( subbus_subfunction == SB_SYSCON104 ) {
    unsigned short cfg_word = ( in16( 0x312 ) & ~mask );
    out16( 0x312, cfg_word );
  }
}

void expint_init( int coid, short code, int value ) {
  expint_iid = int_init( expint_irq, 0x20, 0, coid, code, value );
}

void expint_reset(void) {
  int_reset( expint_iid, 0x3F );
}

void expint_svc(void) {
  service_expint();
  InterruptUnmask( expint_irq, expint_iid );
}

#ifdef IRQ_SUPPORT
  static int spare_iid = -1;
  static int pfail_iid = -1;
  int spare_irq = 0;
  int pfail_irq = 0;

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
#endif
