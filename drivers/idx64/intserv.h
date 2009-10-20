/* intserv.h Defines interfaces for intserv program */
#ifndef INTSERV_H_INCLUDED
#define INTSERV_H_INCLUDED

#include <sys/types.h>

/* This is the API: */
// extern int IntSrv_IRQ_attach( const char *cardID, int IRQ, short pulse_code );
// extern int IntSrv_IRQ_detach( const char *cardID, int IRQ );
extern int IntSrv_Int_attach( const char *cardID, unsigned short address,
						int region, short pulse_code );
extern int IntSrv_Int_detach( char *cardID );

#define ISRV_REGION_A 0x40
#define ISRV_REGION_B 0x42
#define ISRV_REGION_C 0x44
#define ISRV_REGION_D 0x46
//#define ISRV_IRQ_SPARE 0
//#define ISRV_IRQ_PFAIL 1
//#define ISRV_MAX_IRQS 2

#endif
