/* intserv.c
   Interrupt handler for Syscon104
*/
#include <errno.h> /* for errno */
#include <string.h> /* for strerror */
#include <unistd.h> /* for getnid() */
#include <stdlib.h> /* for stroul() */
#include "nortlib.h"
#include "oui.h"
#include "intserv.h"
#include "intserv_int.h"
#include "subbus.h"

#ifdef INTSERV_STANDALONE
  int intserv_quit = 0;

  int main( int argc, char **argv ) {
    int name_id, subbus_id, done = 0;
    
    oui_init_options( argc, argv );
    if ( intserv_quit ) {
      pid_t isrv;
      
      /* request running server to quit */
      isrv = qnx_name_locate( getnid(), ISRV_NAME, 0, 0 );
      if ( isrv == -1 ) {
        nl_error( 3, "Quit request failed" );
      } else {
        msg_t type;
        type = ISRV_QUIT;
        errno = EOK;
        if ( Send( isrv, &type, &type, sizeof( type ), sizeof( type ) )
                != 0 || type != EOK )
          nl_error( 3, "Error requesting quit" );
        nl_error( 0, "Quit request acknowledged" );
      }
      return 0;
    }

    subbus_id = load_subbus();
    expint_init();
    name_id = qnx_name_attach( 0, ISRV_NAME );
    if ( name_id == -1 )
      nl_error( 3, "Error attaching name" );
    errno = EOK;

    while ( ! done ) {
      pid_t who;
      IntSrv_msg buf;
      IntSrv_reply rep;

      who = Receive( 0, &buf, sizeof( buf ) );
      if ( who == -1 ) nl_error( 1, "Receive error" );
      else if ( who == expint_proxy ) {
        service_expint();
      } else if ( who == pfail_proxy ) {
        pfail_proxy_handler( 1 );
      } else if ( who == spare_proxy ) {
        irq_proxy_handler( ISRV_IRQ_SPARE );
      } else {
        rep.status = EOK;
        switch ( buf.type ) {
          case ISRV_QUIT:
            errno = EOK;
            nl_error( 0, "Quit request received" );
            done = 1;
            break;
          case ISRV_INT_ATT:
            expint_attach( who, buf.cardID, buf.address, buf.u.region, 
                          buf.proxy, &rep );
            break;
          case ISRV_INT_DET:
            expint_detach( who, buf.cardID, &rep );
            break;
          case ISRV_IRQ_ATT:
            irq_attach( who, buf.cardID, buf.u.irq, buf.proxy, &rep );
            break;
          case ISRV_IRQ_DET:
            irq_detach( who, buf.cardID, buf.u.irq, &rep );
            break;
          default:
            rep.status = ENOSYS;
            break;
        }
        if ( Reply( who, &rep, sizeof( rep ) ) != 0 )
          nl_error( 1, "Reply error" );
      }
    }
    errno = EOK;
    expint_reset();
    qnx_name_detach( 0, name_id );
    nl_error( 0, "Terminating" );
    return 0;
  }
#endif

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
