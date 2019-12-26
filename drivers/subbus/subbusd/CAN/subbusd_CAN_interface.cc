/*
 @File subbusd_CAN_interface.cc
 */
#include <string.h>
#include <fcntl.h>
#include <errno.h>
// #include "subbusd_CAN_config.h"
#include "subbusd_CAN.h"
// #include "nl.h"
#include "nl_assert.h"
// #include "dasio/ascii_escape.h"
#include "msg.h"
#ifdef HAVE_LINUX_CAN_H
  #include <sys/ioctl.h>
  #include <sys/socket.h>
  #include <linux/can.h>
  #include <linux/can/raw.h>
  #include <linux/can/error.h>
  #include <net/if.h>
#endif

CAN_interface::CAN_interface() :
    request_processing(false),
    req_no(0)
{
  iface = new CAN_serial(this);
}

CAN_interface::~CAN_interface() {}

void CAN_interface::setup() {
  iface->setup();
}

void CAN_interface::enqueue_request(can_msg_t *can_msg, uint8_t *rep_buf, int buflen,
        subbusd_CAN_client *clt) {
  nl_assert(can_msg);
  msg(MSG_DBG(0), "enqueuing: %d", reqs.size());
  reqs.push_back(can_request(can_msg, rep_buf, buflen, clt));
  process_requests();
}

can_request CAN_interface::curreq() {
  nl_assert(!reqs.empty());
  return reqs.front();
}

/**
 * Called
 *  - When a new request has been enqueued
 *  - When a previous request has been completed
 *  - When the current request's output has been flushed and more
 *    may be required (req.msg->sb_can_seq > 0)
 */
void CAN_interface::process_requests() {
  if (iface->request_pending || request_processing || reqs.empty()) {
    msg(MSG_DBG(0), "process_requests() no action: %s",
      iface->request_pending ? "pending" : request_processing ? "processing"
      : "reqs.empty()");
    return;
  }
  request_processing = true;
  can_request req = reqs.front();
  /* A single request might require multiple packets */
  while (!iface->request_pending && iface->obuf_clear()) {
    uint8_t req_seq_no = req.msg->sb_can_seq;
    uint16_t offset = req_seq_no ? (req_seq_no*7 - 1) : 0;
    nl_assert(offset < req.msg->sb_nb);
    uint16_t nbdata = req.msg->sb_nb - offset;
    iface->reqfrm.can_id = CAN_REQUEST_ID(req.msg->device_id,req_no);
    if (req.msg->sb_can_seq) {
      if (nbdata > 7) nbdata = 7;
      iface->reqfrm.can_dlc = nbdata+1;
      iface->reqfrm.data[0] = CAN_CMD(req.msg->sb_can_cmd,req.msg->sb_can_seq);
      memcpy(&iface->reqfrm.data[1], &req.msg->sb_can[offset], nbdata);
    } else {
      iface->rep_recd = 0;
      if (nbdata > 6) nbdata = 6;
      iface->reqfrm.can_dlc = nbdata+2;
      iface->reqfrm.data[0] = CAN_CMD(req.msg->sb_can_cmd,req_seq_no);
      iface->reqfrm.data[1] = req.msg->sb_nb;
      memcpy(&iface->reqfrm.data[2], &req.msg->sb_can[offset], nbdata);
    }
    ++req.msg->sb_can_seq;
    if (offset+nbdata >= req.msg->sb_nb) {
      iface->request_pending = true;
      ++req_no;
    }
    iface->rep_seq_no = 0;
    if (nl_debug_level <= MSG_DBG(1)) {
      char msgbuf[80];
      int nc = 0;
      nc += snprintf(&msgbuf[nc], 80-nc, "CANout ID:x%02X Data:", iface->reqfrm.can_id);
      for (int i = 0; i < iface->reqfrm.can_dlc; ++i) {
        nc += snprintf(&msgbuf[nc], 80-nc, " %02X", iface->reqfrm.data[i]);
      }
      msg(MSG_DBG(1), "%s", msgbuf);
    }
    if (iface->send_packet()) return;
#if 0
    if (!iface->obuf_clear()) {
      iface->report_err("%s: process_requests() !obuf_empty() after iwrite",
        iface->get_iname());
      iface->iwrite_cancel();
      reqs.pop_front();
      memset(req.msg->buf, 0, req.msg->bufsz-iface->rep_recd);
      req.clt->request_complete(SBS_NOACK, req.msg->bufsz);
      // req.clt->request_complete(SBS_TIMEOUT, 0);
      iface->request_pending = false;
      request_processing = false;
      return;
    }
#endif
    if (iface->request_pending) {
      msg(MSG_DBG(1), "%s: Setting timeout", iface->get_iname());
      iface->TO.Set(0,50);
      // iface->flags |= DAS_IO::Interface::Fl_Timeout;
    } else {
      msg(MSG_DBG(1), "%s: Request resolved immediately", iface->get_iname());
    }
  }
  request_processing = false;
}

const char *CAN_serial::port = "/dev/ttyS1";
uint32_t CAN_serial::baud_rate = 57600;

CAN_serial::CAN_serial(CAN_interface *parent)
  : sb_interface("slcan", 80),
    request_pending(false),
    parent(parent),
    slcan_state(st_init)
    {}

void CAN_serial::setup() {
#if 0
  init(port, O_RDWR | O_NONBLOCK);
  DAS_IO::Serial::setup(baud_rate, 8, 'n', 1, 6, 0);
  flush_input();
  issue_init();
  update_tc_vmin(30);
  flags |= DAS_IO::Interface::Fl_Timeout;
#endif
  issue_init();
  update_tc_vmin(30);
}

void CAN_serial::cleanup() {
  if (obuf_empty()) {
    iwrite("C\r");
    msg(0, "%s: CAN %s", iname, obuf_empty() ? "closed" : "almost closed");
  } else {
    msg(1, "%s: cleanup() and !obuf_empty()");
  }
}

void CAN_serial::issue_init() {
  msg(MSG_DBG(1),"%s: Sending init string", iname);
  iwrite("C\rS2\rO\rV\r");
  // iwrite(init_strings[n_init++]);
  TO.Set(0,100);
}

bool CAN_serial::send_packet() {
  int ocp = 0;
  ocp += snprintf(&obuf[ocp], obufsize, "t%03X%X", reqfrm.can_id, reqfrm.can_dlc);
  for (int i = 0; i < reqfrm.can_dlc; ++i)
    ocp += snprintf(&obuf[ocp], obufsize-ocp, "%02X", reqfrm.data[i]);
  nl_assert(ocp < obufsize);
  obuf[ocp++] = '\r';
  // msg(MSG_DBG(1), "SLCANout: %s", ::ascii_escape(obuf, ocp));
  msg(MSG_DBG(1), "SLCANout: %s", ::ascii_escape(obuf));
  update_tc_vmin(5); // tAAAL
  return iwrite(&obuf[0], ocp);
}

bool CAN_serial::iwrite(const char *str, int nc) {
  int rv = write(fd, str, nc);
  return rv < 0;
}

bool CAN_serial::iwrite(const char *str) {
  int nc = strlen(str);
  return iwrite(str, nc);
}

bool CAN_serial::iwritten(int nb) {
  if (obuf_empty() && !request_pending && slcan_state == st_operate) {
    parent->process_requests();
  }
}

bool CAN_serial::protocol_input() {
  struct can_frame rep_frame;
  struct can_frame *repfrm = &rep_frame;
  uint16_t can_id, can_len, can_data;
  
  // parse text into rep_frame, then process identically to CAN_socket
  // start processing only 't' responses
  if (nc == 0) return false;
  if (slcan_state == st_operate) {
    if (buf[0] != 't') {
      report_err("%s: Unexpected input type", iname);
      consume(nc);
      return false;
    }
    ++cp;
    if (not_nhexdigits(3, can_id) ||
        not_nhexdigits(1, can_len)) {
      if (cp < nc) {
        consume(nc);
      }
      update_tc_vmin(5-nc);
      return false;
    }
    rep_frame.can_id = can_id;
    rep_frame.can_dlc = can_len;
    int expected_total_chars = 5 + 2+can_len + 1;
    int i;
    for (i = 0; i < can_len && !not_nhexdigits(2, can_data); ++i) {
      rep_frame.data[i] = can_data;
    }
    if (i < can_len) {
      if (cp < nc) {
        consume(nc);
      }
      update_tc_vmin(expected_total_chars - nc);
      return false; // wait for more chars
    }
    update_tc_vmin(1); // we've received everything we need
    
    // reassemble longer response as necessary
    if (!request_pending) {
      report_err("%s: Unexpected input", iname);
      consume(nc);
      return false;
    }
    can_request request = parent->curreq();
    // check for CAN error frame
    if (repfrm->can_id & (CAN_EFF_FLAG|CAN_RTR_FLAG)) {
      report_err("%s: Unexpected packet type: ID:%08X", iname, repfrm->can_id);
      consume(nc);
      return false;
    }
    if (repfrm->can_id & CAN_ERR_FLAG) {
      report_err("%s: CAN error frame ID:0x%X", iname, repfrm->can_id & CAN_ERR_MASK);
      consume(nc);
      return false;
    }
    // check incoming ID with request
    if ((repfrm->can_id & CAN_SFF_MASK) !=
        ((reqfrm.can_id & CAN_SFF_MASK) | CAN_ID_REPLY_BIT)) {
      report_err("%s: Invalid ID: %X, expected %X", iname,
        repfrm->can_id, reqfrm.can_id | CAN_ID_REPLY_BIT);
      consume(nc);
      return false;
    }
    // check incoming cmd with request
    // check incoming seq with req_seq_no
    if (repfrm->can_dlc < 2) {
      report_err("%s: DLC:%d (<2)", iname, repfrm->can_dlc);
      consume(nc);
      return false;
    }
    if (repfrm->data[0] != CAN_CMD(reqfrm.data[0],rep_seq_no)) {
      if (CAN_CMD_CODE(repfrm->data[0]) == CAN_CMD_CODE_ERROR) {
        if (repfrm->data[2] == CAN_ERR_NACK) {
          memset(request.msg->buf, 0, request.msg->bufsz - rep_recd);
          request.clt->request_complete(SBS_NOACK, request.msg->bufsz);
        } else {
          report_err("%s: CAN_ERR %d", iname, repfrm->data[1]);
          request.clt->request_complete(SBS_RESP_ERROR, 0);
        }
      } else {
        report_err("%s: req/rep cmd,seq mismatch: %02X/%02X",
          iname, repfrm->data[0], reqfrm.data[0]);
        consume(nc);
        return false;
      }
      parent->pop_req();
      // reqs.pop_front();
      request_pending = false;
      report_ok(nc);
      TO.Clear();
      parent->process_requests();
      return false;
    }
    // if seq == 0, check len with request and update
    int nbdat = repfrm->can_dlc - 1; // not counting cmd byte
    uint8_t *data = &repfrm->data[1];
    if (CAN_CMD_SEQ(repfrm->data[0]) == 0) {
      rep_len = repfrm->data[1];
      if (rep_len > request.msg->bufsz) {
        report_err("%s: reply length %d exceeds request len %d",
          iname, rep_len, request.msg->bufsz);
        consume(nc);
        return false;
      }
      --nbdat;
      ++data;
      msg(MSG_DBG(2), "rep_recd: %d", rep_recd);
    }
    // check dlc_len against remaining request len
    if (rep_recd + nbdat > rep_len) {
      report_err("%s: msg overflow. cmdseq=%02X dlc=%d rep_len=%d",
        iname, repfrm->data[0], repfrm->can_dlc, rep_len);
      consume(nc);
      return false;
    }
    if (nl_debug_level <= MSG_DBG(1)) {
      msg(MSG_DBG(1), "CANin %s", ascii_escape());
    }
    
    // copy data into reply
    memcpy(request.msg->buf, data, nbdat);
    request.msg->buf += nbdat;
    rep_recd += nbdat;
    msg(MSG_DBG(2), "Seq:%d nbdat:%d recd:%d rep_len:%d",
      rep_seq_no, nbdat, rep_recd, rep_len);
    // update rep_seq_no
    ++rep_seq_no;
    report_ok(nc);
    msg(MSG_DBG(1), "%s: Clearing timeout", iname);
    TO.Clear();
    // If request is complete, call clt->request_complete
    if (rep_recd == rep_len) {
      //reqs.pop_front();
      parent->pop_req();
      // clearing request_pending after request_complete()
      // simply limits the depth of recursion
      request.clt->request_complete(SBS_ACK, rep_len);
      request_pending = false;
      parent->process_requests();
    }
  } else if (slcan_state == st_init || slcan_state == st_init_retry) {
    if (buf[nc-1] == '\r' || buf[nc-1] == '\n') {
      msg(0, "ID: %s", ascii_escape());
      TO.Clear();
      slcan_state = st_operate;
      consume(nc);
    }
  } else {
    report_err("%s: Unexpected input in state %d", iname, slcan_state);
  }
  return false;
}

bool CAN_serial::protocol_timeout() {
  TO.Clear();
  if (request_pending) {
    can_request request = parent->curreq(); // reqs.front();
    msg(MSG_DBG(0), "%s: Timeout reading from ID:0x%X", iname,
      request.msg->device_id);
    consume(nc);
    parent->pop_req(); // reqs.pop_front();
    memset(request.msg->buf, 0, request.msg->bufsz-rep_recd);
    request.clt->request_complete(SBS_NOACK, request.msg->bufsz);
    request_pending = false;
    parent->process_requests();
  } else if (slcan_state == st_init) {
    msg(1, "%s: Init Timeout: nc=%d '%s'", iname, nc,
      // ::ascii_escape((const char *)buf, nc));
      ::ascii_escape((const char *)buf));
    TO.Set(4,0);
    slcan_state = st_init_retry;
    consume(nc);
  } else if (slcan_state == st_init_retry) {
    slcan_state = st_init;
    issue_init();
  } else {
    msg(1, "%s: Timeout without request_pending", iname);
  }
  return false;
}

void subbus_timeout() {
  subbusd_CAN::CAN_flavor->CAN_timeout();
}

void CAN_serial::update_tc_vmin(int new_vmin) {
  if (! termios_init) {
    if (tcgetattr(fd, &ss_termios)) {
      nl_error(2, "Error from tcgetattr: %s",
        strerror(errno));
    }
    termios_init = true;
  }
  if (new_vmin < 1) new_vmin = 1;
  if (new_vmin != ss_termios.c_cc[VMIN]) {
    ss_termios.c_cc[VMIN] = new_vmin;
    if (tcsetattr(fd, TCSANOW, &ss_termios)) {
      nl_error(2, "Error from tcsetattr: %s",
        strerror(errno));
    }
  }
}

bool CAN_serial::closed() {
  msg(0, "%s: serial port closed!!", iname);
  return true;
}
