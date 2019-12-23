#include "subbusd_int.h"

/**
 * The default implementation allows using the same buffer for all
 * clients. This is fine in most cases because the library requests
 * map 1:1 onto device requests. The assumption fails with CAN, where
 * multiread library requests may need to be mapped onto more than
 * one subbus_CAN protocol request, and subbus_CAN requests themselves
 * can map to more than one CAN request. Hence the subbusd_CAN
 * driver needs to override this function to provide client-specific
 * buffers.
 */
subbusd_req_t *get_client_buffer(int rcvid) {
  static subbusd_req_t req;
  rcvid = rcvid;
  return &req;
}
