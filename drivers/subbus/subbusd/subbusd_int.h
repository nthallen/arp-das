#ifndef SUBBUSD_INT_H_INCLUDED
#define SUBBUSD_INT_H_INCLUDED
#include <sys/dispatch.h>
#include "subbusd.h"

/** \file
  * \brief Defines interface between QNX6 resource manager and subbus hardware
  *
  * A different hardware architecture can be supported by reimplementing these
  * functions.
  */

typedef struct carddef {
  struct carddef *next;
  char cardID[ CardID_MAX ];
  unsigned short address;
  unsigned int reg_id;
  unsigned int bitno;
  struct sigevent event;
  int owner;
} card_def;

typedef struct {
  unsigned short address;
  unsigned short bits;
  card_def *def[8];
} region;
#define MAX_REGIONS 4

extern const char *subbusd_devname;
extern card_def *carddefs;
extern region regions[MAX_REGIONS];
extern void incoming_sbreq( int rcvid, subbusd_req_t *req );
extern void init_subbus(dispatch_t *dpp );
extern void shutdown_subbus(void);
extern int expint_attach( int rcvid, char *cardID, unsigned short address,
                      int region, struct sigevent *event, card_def **card );
extern int expint_detach( int rcvid, char *cardID, unsigned short *addr,
                      unsigned int *bn );

extern char *cache_hw_range, *cache_sw_range;
extern void sb_cache_init(void);
extern int sb_cache_write( unsigned short addr, unsigned short data );
extern int sb_cache_read( unsigned short addr, unsigned short *data );

#endif
