#ifndef SUBBUSD_CAN_INTERFACE_H_INCLUDED
#define SUBBUSD_CAN_INTERFACE_H_INCLUDED

#include <list>
#include "dasio/interface.h"
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

#ifdef USE_CAN_SOCKET
  class CAN_socket;
  #ifdef HAVE_LINUX_CAN_H
    #define SUBBUSD_CAN_NAME "le-das CAN driver V1.1"
  #else
    #define SUBBUSD_CAN_NAME "le-das CANSIM driver V1.1"
  #endif
#endif

#ifdef USE_SLCAN
  #define SUBBUSD_CAN_NAME "le-das SLCAN driver V1.1"
  #include "dasio/serial.h"
  class CAN_serial;
#endif

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
  uint8_t sb_nb;
  uint8_t sb_can[256];
  bool end_of_request;
  uint8_t *buf;
  int bufsz;
} can_msg_t;

class can_request;
class subbusd_CAN_client;
class CAN_interface;

#ifdef USE_CAN_SOCKET

  class CAN_socket : public DAS_IO::Interface {
    friend class CAN_interface;
    public:
      CAN_socket(CAN_interface *parent);
      void setup();
      inline void cleanup() {}
      bool send_packet();
      inline bool obuf_clear() { return obuf_empty(); }
      bool request_pending;
    protected:
      bool iwritten(int nb);
      const char *ascii_escape();
      bool protocol_input();
      bool protocol_timeout();
      bool closed();
      uint8_t rep_seq_no;
      uint16_t rep_len;
      uint16_t rep_recd;
      can_frame reqfrm;
      CAN_interface *parent;
  };

#endif

#ifdef USE_SLCAN

  class CAN_serial : public DAS_IO::Serial {
    friend class CAN_interface;
    public:
      CAN_serial(CAN_interface *parent);
      void setup();
      void cleanup();
      void issue_init();
      bool send_packet();
      inline bool obuf_clear() { return obuf_empty(); }
      bool request_pending;
      static const char *port;
      static uint32_t baud_rate;
    protected:
      bool iwritten(int nb);
      bool protocol_input();
      bool protocol_timeout();
      bool closed();
      static const int obufsize = 24;
      char obuf[obufsize];
      uint8_t rep_seq_no;
      uint16_t rep_len;
      uint16_t rep_recd;
      can_frame reqfrm;
      CAN_interface *parent;
      enum { st_init, st_init_retry, st_operate } slcan_state;
  };

#endif

class CAN_interface {
  #ifdef USE_CAN_SOCKET
    friend class CAN_socket;
  #endif
  #ifdef USE_SLCAN
    friend class CAN_serial;
  #endif
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
    inline void reference() { iface->reference(); }
    inline void dereference() { DAS_IO::Interface::dereference(iface); }
    inline DAS_IO::Interface *iface_ptr() { return iface; }
  protected:
  private:
    void process_requests();
    std::list<can_request> reqs;
    bool request_processing;
    uint8_t req_no;
    
    #ifdef USE_CAN_SOCKET
      #ifndef HAVE_LINUX_CAN_H
        uint8_t bytectr;
      #endif
      CAN_socket *iface;
    #endif
    
    #ifdef USE_SLCAN
      CAN_serial *iface;
    #endif
};

#endif
