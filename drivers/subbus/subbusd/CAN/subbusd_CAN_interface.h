#ifndef SUBBUSD_CAN_INTERFACE_H_INCLUDED
#define SUBBUSD_CAN_INTERFACE_H_INCLUDED

#ifdef __cplusplus

#include <stdint.h>
#include <list>
#include <termios.h>
#include "msg.h"
#include "timeout.h"
// #include "dasio/interface.h"

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

#define SUBBUSD_CAN_NAME "le-das SLCAN driver V1.1"
// #include "dasio/serial.h"
class CAN_serial;

/**
 * While message is being processed, sb_can_seq will be
 * incremented. Since there are 6 bytes of data in the
 * first frame and 7 bytes in each frame after that, the
 * starting offset by sb_can_seq is:
 *   0 => 0
 *   1 => 6
 *   2 => 13
 * offset = sb_can_seq ? 7*sb_can_seq - 1 : 0;
 */
typedef struct {
  uint8_t device_id;
  uint8_t sb_can_cmd;
  uint8_t sb_can_seq;
  uint8_t sb_nb; // length of subbus_CAN message
  uint8_t sb_can[256]; // Single subbus_CAN message
  bool end_of_request;
  uint8_t *buf; // reply buffer
  int bufsz;
} can_msg_t;

class can_request;
class subbusd_CAN_client;
class CAN_interface;

class CAN_serial : public sb_interface /* : public DAS_IO::Serial */ {
  friend class CAN_interface;
  public:
    CAN_serial(CAN_interface *parent);
    void setup();
    void cleanup();
    void issue_init();
    bool send_packet();
    inline bool obuf_empty() { return true; }
    inline bool obuf_clear() { return obuf_empty(); }
    unsigned char *get_buf() { return buf; }
    bool request_pending;
    static const char *port;
    static uint32_t baud_rate;
  protected:
    bool iwrite(const char *str, int nc);
    bool iwrite(const char *str);
    bool iwritten(int nb);
    bool protocol_input();
    bool protocol_timeout();
    bool closed();
    // void update_tc_vmin(int vmin);
    static const int obufsize = 24;
    char obuf[obufsize];
    uint8_t rep_seq_no;
    uint16_t rep_len;
    uint16_t rep_recd;
    can_frame reqfrm;
    Timeout TO;
    CAN_interface *parent;
    enum { st_init, st_init_retry, st_operate } slcan_state;
    bool termios_init;
    termios ss_termios;
};

class CAN_interface {
  friend class CAN_serial;
  public:
    CAN_interface();
    ~CAN_interface();
    /** Open and setup the CAN socket */
    void setup();
    inline void cleanup() { if (iface) iface->cleanup(); }
    void enqueue_request(can_msg_t *can_msg, uint8_t *rep_buf, int buflen,
        subbusd_CAN_client *clt);
    can_request curreq();
    inline void pop_req() { reqs.pop_front(); }
    inline unsigned char *get_buf() { return iface->get_buf(); }
    // inline void reference() { iface->reference(); }
    // inline void dereference() { DAS_IO::Interface::dereference(iface); }
    inline CAN_serial *iface_ptr() { return iface; }
    inline bool protocol_input() { return iface->protocol_input(); }
    inline bool protocol_timeout() { return iface->protocol_timeout(); }
  protected:
  private:
    void process_requests();
    std::list<can_request> reqs;
    bool request_processing;
    uint8_t req_no;
    CAN_serial *iface;
};

extern "C" {
#endif

extern void subbus_timeout();

#ifdef __cplusplus
}
#endif

#endif
