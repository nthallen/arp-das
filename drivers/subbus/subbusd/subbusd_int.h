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
extern void incoming_sbreq( int rcvid, char *req );
extern void init_subbus(dispatch_t *dpp );
extern void shutdown_subbus(void);

#endif
