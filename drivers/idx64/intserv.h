/* intserv.h Defines interfaces for intserv program */
#ifndef INTSERV_H_INCLUDED
#define INTSERV_H_INCLUDED

#include <sys/types.h>
#define cardID_MAX 35

/* This is the API: */

#define ISRV_REGION_A 0x40
#define ISRV_REGION_B 0x42
#define ISRV_REGION_C 0x44
#define ISRV_REGION_D 0x46


extern int expint_irq;
extern void process_IRQs( char *s );
extern int intserv_quit;
extern void expint_init( int coid, short code, int value );
extern void expint_reset( void );
extern void spare_init( void );
extern void spare_reset( void );
extern void pfail_init( void );
extern void pfail_reset( void );
extern void service_expint( void );
extern int expint_attach( /* pid_t who, */ const char *cardID,
    unsigned short address, int region, short code, int value ); 
extern int expint_detach( /* pid_t who, */ const char *cardID );

#ifdef IRQ_SUPPORT
  #define ISRV_IRQ_SPARE 0
  #define ISRV_IRQ_PFAIL 1
  #define ISRV_MAX_IRQS 2
  extern int spare_irq;
  extern int pfail_irq;
  extern void irq_attach( /* pid_t who, */ const char *cardID, short irq,
      pid_t proxy, IntSrv_reply *rep );
  extern void irq_detach( /* pid_t who, */ const char *cardID, short irq,
      IntSrv_reply *rep );
  extern void pfail_proxy_handler( int saw_irq );
  extern void irq_proxy_handler( int irq );
#endif

#endif
