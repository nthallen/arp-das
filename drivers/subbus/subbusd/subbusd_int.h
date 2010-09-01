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
  char cardID[ cardID_MAX ];
  unsigned short address;
  unsigned int reg_id;
  unsigned int bitno;
  struct sigevent event;
  pid_t owner;
} card_def;

extern card_def *carddefs;
extern void incoming_sbreq( int rcvid, subbusd_req_t *req );
extern void init_subbus(dispatch_t *dpp );
extern void shutdown_subbus(void);
extern card_def *expint_attach( int rcvid, char *cardID, unsigned short address,
                      int region, struct sigevent *event, IntSrv_reply *rep );
extern int expint_detach( int rcvid, char *cardID, unsigned short *addr, unsigned int *bn );

#endif
