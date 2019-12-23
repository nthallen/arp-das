#ifndef SB_CAN_H_INCLUDED
#define SB_CAN_H_INCLUDED
#include "subbusd_int.h"

/**
 SB_CAN_MAX_REQUEST is the maximum allowable string
 for a serialized command over the USB.
 The longest request will be for the multi-read
 command, and we may see fit to extend this size.

 The multi-read response may also be even longer, but
 we do not necessarily have to allocate buffer space for
 the entire response, since we can process it as it comes
 in.
 */
#define SB_CAN_MAX_REQUEST 256
#define SB_CAN_MAX_RESPONSE 2501

#if 0
// I believe I will replace all of this
typedef struct {
  int type;
  int status;
  int rcvid;
  unsigned short n_reads;
  char request[SB_CAN_MAX_REQUEST];
} sbd_request_t;

#define SBDR_TYPE_INTERNAL 0
#define SBDR_TYPE_CLIENT 1
#define SBDR_TYPE_MAX 1
#define SBDR_STATUS_QUEUED 0
#define SBDR_STATUS_SENT 1

#endif // 0

/* SUBBUSD_MAX_REQUESTS is the size of the request queue,
   so it determines the number of simultaneous requests 
   we can handle. Current usage suggests we will have
   a small number of programs accessing the subbus
   (col,srvr,idxr,dccc,ana104,card,digital) so 20 is
   not an unreasonable upper bound */
#define SUBBUSD_MAX_REQUESTS 20

// extern int int_attach(int rcvid, subbusd_req_t *req, char *sreq);
// extern int int_detach(int rcvid, subbusd_req_t *req, char *sreq);

extern const char *CAN_port;
extern int CAN_baud_rate;

#endif
