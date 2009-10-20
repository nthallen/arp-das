/* Definitions of functions that are needed internally by the
   server
*/
#ifndef IS_INTERNAL_INCLUDED
#define IS_INTERNAL_INCLUDED

#include "intserv.h"

extern pid_t expint_proxy;
//extern pid_t spare_proxy;
//extern pid_t pfail_proxy;
extern int expint_irq;
//extern int spare_irq;
//extern int pfail_irq;
extern void process_IRQs( char *s );
extern int intserv_quit;
extern void expint_init( int coid, short code, int value );
extern void expint_reset( void );
extern void spare_init( void );
extern void spare_reset( void );
extern void pfail_init( void );
extern void pfail_reset( void );
//extern pid_t far expint_handler( void );
//extern pid_t far spare_handler( void );
//extern pid_t far pfail_handler( void );
extern void service_expint( void );
extern int expint_attach( /* pid_t who, */ const char *cardID,
    unsigned short address, int region, short code, int value ); 
extern int expint_detach( /* pid_t who, */ const char *cardID );
#ifdef IRQ_SUPPORT
  extern void irq_attach( /* pid_t who, */ const char *cardID, short irq,
					    pid_t proxy, IntSrv_reply *rep );
  extern void irq_detach( /* pid_t who, */ const char *cardID, short irq, IntSrv_reply *rep );
  extern void pfail_proxy_handler( int saw_irq );
  extern void irq_proxy_handler( int irq );
#endif

#endif
