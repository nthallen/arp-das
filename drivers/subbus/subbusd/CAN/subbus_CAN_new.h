
#ifdef HAVE_LINUX_CAN_H
  #include <linux/can.h>
  #include <linux/can/raw.h>
#else
  // placeholders for testing on unsupported platforms (cygwin)
  // and also for SLCAN
  struct can_frame {
    uint32_t can_id;
    uint8_t  can_dlc;
    uint8_t  data[8];
  };
  #define CAN_MTU sizeof(struct can_frame)
  #define CAN_SFF_MASK 0x000007FFU
  #define CAN_EFF_FLAG 0x80000000U
  #define CAN_RTR_FLAG 0x40000000U
  #define CAN_ERR_FLAG 0x20000000U
  #define CAN_ERR_MASK 0x1FFFFFFFU
#endif

#define SB_CAN_MAX_REQUEST 256
#define SB_CAN_MAX_RESPONSE 80

/**
 * An incoming_sbreq() may be satisfied with a single CAN
 * packet (particularly single-word read/write operations).
 *
 * Multi-read operations are translated into one or more
 * subbus_CAN messages. The subbus_CAN messages in turn
 * can consist of one or more CAN packets (but only in
 * the multi-read case. The primitive operations all fit
 * into a single CAN packet.
 *
 * We will copy the mread request into the mread field.
 *
 * While message is being processed, sb_can_seq will be
 * incremented. Since there are 6 bytes of data in the
 * first frame and 7 bytes in each frame after that, the
 * starting offset by sb_can_seq is:
 *   0 => 0
 *   1 => 6
 *   2 => 13
 * offset = sb_can_seq ? 7*sb_can_seq - 1 : 0;
 *
 * This is the structure that makes up the request queue.
 * If mr_nc > 0, this is a multi-read request, and sb_can_cmd
 * may not yet be defined. Otherwise, sb_can_cmd defines the
 * command, which can be encoded into a single CAN packet.
 */
typedef struct {
  // int type;
  int status;
  int rcvid;
  // unsigned short n_reads;
  // char request[SB_SERUSB_MAX_REQUEST];
  uint8_t device_id;
  uint8_t sb_can_cmd;
  uint8_t sb_can_seq;
  uint8_t mread[256];
  int mr_nc, mr_cp;
  uint8_t sb_can[256];
  uint8_t sb_nc, sb_cp;
  bool end_of_request;
} can_request_t;

#define SBDR_TYPE_INTERNAL 0
#define SBDR_TYPE_CLIENT 1
#define SBDR_TYPE_MAX 1

#define SBDR_STATUS_FREE 0
#define SBDR_STATUS_ALLOCATED 1
#define SBDR_STATUS_ENQUEUED 2
#define SBDR_STATUS_SENT 3
