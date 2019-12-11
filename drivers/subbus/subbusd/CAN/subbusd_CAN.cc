// #undef HAVE_CAN_H
#include <string.h>
#include <fcntl.h>
#include "subbusd_CAN_config.h"
#include "nl_assert.h"
#include "subbusd_int.h"
#include "subbusd_CAN.h"
#include "dasio/ascii_escape.h"

using namespace DAS_IO;

subbusd_CAN_client::subbusd_CAN_client(DAS_IO::Authenticator *auth, subbusd_CAN *fl)
    : subbusd_client(auth), flavor(fl) {}
subbusd_CAN_client::~subbusd_CAN_client() {}

Serverside_client *new_subbusd_CAN_client(Authenticator *auth, SubService *ss) {
  ss = ss; // not interested
  subbusd_CAN_client *clt =
    new subbusd_CAN_client(auth, (subbusd_CAN*)ss->svc_data);
  return clt;
}

/*
 * setup reply structure, along with maximum reply size,
 * bytes read. The reply size obviously depends on the command,
 * as does where returned data is reported in the reply.
 */
bool subbusd_CAN_client::incoming_sbreq() {
  int rv, rsize;
  //uint8_t device_id, addr;
  // req = (subbusd_req_t *)&buf[0];
  
  switch ( req->sbhdr.command ) {
    case SBC_READACK:
      rep.hdr.ret_type = SBRT_US;
      can_msg.device_id = (req->data.d1.data >> 8) & 0xFF;
      can_msg.sb_can_cmd = CAN_CMD_CODE_RD;
      can_msg.sb_can_seq = 0;
      can_msg.sb_nb = 1;
      can_msg.sb_can[0] = req->data.d1.data & 0xFF;
      can_msg.end_of_request = true;
      flavor->enqueue_request(&can_msg, (uint8_t*)&rep.data.value, 2, this);
      break;
    case SBC_MREAD:
      // Setup necessary preconditions, then call processing function
      // Need to decode this and enqueue multiple requests
      // enqueue_sbreq(device_id, req->data.d4.multread_cmd,
      //              req->data.d4.n_reads);
      setup_mread();
      break;
    case SBC_WRITEACK:
      rep.hdr.ret_type = SBRT_NONE;
      can_msg.device_id = (req->data.d0.address >> 8) & 0xFF;
      can_msg.sb_can_cmd = CAN_CMD_CODE_WR_INC;
      can_msg.sb_can_seq = 0;
      can_msg.sb_nb = 3;
      can_msg.sb_can[0] = req->data.d0.address & 0xFF;
      can_msg.sb_can[1] = req->data.d0.data & 0xFF;
      can_msg.sb_can[2] = (req->data.d0.data >> 8) & 0xFF;
      can_msg.end_of_request = true;
      flavor->enqueue_request(&can_msg, 0, 0, this);
      break;
    case SBC_GETCAPS:
      rep.hdr.status = SBS_OK;
      rep.hdr.ret_type = SBRT_CAP;
      rep.data.capabilities.subfunc = 11; // CAN Driver, defined in Evernote
      rep.data.capabilities.features = 0; // Really none!
      strncpy(rep.data.capabilities.name, SUBBUSD_CAN_NAME, SUBBUS_NAME_MAX);
      return iwrite((const char *)&rep, sizeof(rep));
    case SBC_QUIT:
      status_return(SBS_OK);
      return true; // Is this sufficient?
    default:
      return status_return(SBS_NOT_IMPLEMENTED);
  }
  return false;
}

void subbusd_CAN_client::request_complete(int16_t status, uint16_t n_bytes) {
  rep.hdr.status = status;
  if (status < 0) {
    status_return(status);
    return;
  }
  if (n_bytes & 1) {
    msg(1, "%s: request_complete: n_bytes is odd: %d",
      iname, n_bytes);
  }
  switch (rep.hdr.ret_type) {
    case SBRT_NONE:
      if (n_bytes > 0) {
        report_err("%s: Excess data for SBRT_NONE, nb=%d", iname, n_bytes);
      } else report_ok();
      iwrite((const char *)&rep, sizeof(subbusd_rep_hdr_t));
      break;
    case SBRT_US: // Return unsigned short value
      if (n_bytes != 2) {
        report_err("%s: Expected 2 bytes, received %d", iname, n_bytes);
      } else report_ok();
      iwrite((const char*)&rep, sizeof(subbusd_rep_hdr_t)+2);
      break;
    case SBRT_MREAD: // Multi-Read
    case SBRT_MREADACK: // Multi-Read w/ACK
      if (n_bytes > mread_word_space_remaining*2) {
        report_err("%s: Overrun on mread. n_bytes=%d words_remaining=%d",
          n_bytes, mread_word_space_remaining);
        status_return(SBS_RESP_ERROR);
        return;
      }
      rep.data.mread.n_reads += n_bytes/2;
      if (buf[cp] == '\n') {
        iwrite((const char*)&rep,
          sizeof(subbusd_rep_hdr_t)+2+2*rep.data.mread.n_reads);
        report_ok();
      } else {
        process_mread();
      }
      break;
    case SBRT_CAP:  // Capabilities: We should not see this here
    default:
      report_err("%s: Invalid ret_type %d in request_complete", iname, rep.hdr.ret_type);
      status_return(SBS_REQ_SYNTAX);
      break;
  }
}

void subbusd_CAN_client::setup_mread() {
  cp = ((unsigned char*)&req->data.d4.multread_cmd) - (&buf[0]);
  if (not_str("M") ||
      not_hex(mread_word_space_remaining) ||
      not_str("#")) {
    status_return(SBS_REQ_SYNTAX);
    return;
  }
  if (mread_word_space_remaining != req->data.d4.n_reads) {
    msg(1, "setup_mread() M%X# != n_reads %X",
      mread_word_space_remaining, req->data.d4.n_reads);
  }
  rep.hdr.ret_type = SBRT_MREAD;
  rep.data.mread.n_reads = 0;
  process_mread();
}

/**
 * On entry, buf[cp] should point to [0-9a-fA-F] (start) or ',' (continue)
 * '\n' is also a possibility, but I am handling that in request_complete()
 */
void subbusd_CAN_client::process_mread() {
  uint16_t arg1, arg2, arg3;
  if (rep.data.mread.n_reads > 0 && cp < nc && buf[cp] == ',') {
    ++cp;
  }
  if (not_hex(arg1)) {
    if (cp >= nc)
      report_err("%s: Truncated mread?", iname);
    status_return(SBS_REQ_SYNTAX);
    return;
  }
  switch (buf[cp]) {
    case ',':
      can_msg.sb_can_cmd = CAN_CMD_CODE_RD;
      can_msg.sb_can_seq = 0;
      can_msg.device_id = (arg1>>8) & 0xFF;
      can_msg.sb_can[0] = arg1&0xFF;
      can_msg.sb_nb = 1;
      format_mread_rd();
      break;
    case ':':
      ++cp;
      if (not_hex(arg2) || not_str(":") || not_hex(arg3) || cp >= nc) {
        if (cp >= nc)
          report_err("%s: Truncated mread?", iname);
        status_return(SBS_REQ_SYNTAX);
        return;
      }
      if (arg3 < arg1 || (arg3&0xFF00) != (arg1&0xFF00)) {
        report_err("%s: Invalid RD_INC %X:1:%X", iname, arg1, arg3);
        status_return(SBS_REQ_SYNTAX);
        return;
      }
      if (arg2 == 1) {
        uint8_t count = (arg3-arg1)+1;
        // setup CAN_CMD_CODE_RD_INC
        can_msg.sb_can_cmd = CAN_CMD_CODE_RD_INC;
        can_msg.sb_can_seq = 0;
        can_msg.device_id = (arg1>>8) & 0xFF;
        can_msg.sb_nb = 2;
        can_msg.sb_can[0] = count; // count
        can_msg.sb_can[1] = arg1 & 0xFF;
        can_msg.end_of_request = (buf[cp] == '\n');
        flavor->enqueue_request(&can_msg,
          (uint8_t*)&rep.data.mread.rvals[rep.data.mread.n_reads],
          count*2, this);
      } else {
        // setup CAN_CMD_CODE_RD and unwind
        can_msg.sb_can_cmd = CAN_CMD_CODE_RD;
        can_msg.sb_can_seq = 0;
        can_msg.device_id = (arg1>>8) & 0xFF;
        can_msg.sb_nb = 0;
        for ( ; arg1 <= arg3; arg1 += arg2) {
          can_msg.sb_can[can_msg.sb_nb] = arg1 & 0xFF;
          ++can_msg.sb_nb;
        }
        format_mread_rd();
        break;
      }
      break;
    case '@':
      ++cp;
      if (arg1 > 255) {
        report_err("%s: Invalid count %d in @", iname, arg1);
        status_return(SBS_REQ_SYNTAX);
        return;
      }
      if (not_hex(arg2) || cp >= nc) {
        if (cp >= nc) report_err("%s: Truncated mread? '%s'", iname,
          ::ascii_escape((const char *)req->data.d4.multread_cmd));
        status_return(SBS_REQ_SYNTAX);
        return;
      }
      can_msg.device_id = (arg2>>8) & 0xFF;
      can_msg.sb_can_cmd = CAN_CMD_CODE_RD_NOINC;
      can_msg.sb_can_seq = 0;
      can_msg.sb_can[0] = arg1&0xFF;
      can_msg.sb_can[1] = arg2&0xFF;
      can_msg.sb_nb = 2;
      can_msg.end_of_request = (buf[cp] == '\n');
      flavor->enqueue_request(&can_msg,
          (uint8_t*)&rep.data.mread.rvals[rep.data.mread.n_reads],
          arg1*2, this);
      break;
    case '|':
      ++cp;
      if (not_hex(arg2) || not_str("@") ||
          not_hex(arg3) || cp >= nc) {
        if (cp >= nc) report_err("%s: Truncated mread? '%s'", iname,
          ::ascii_escape(req->data.d4.multread_cmd));
        status_return(SBS_REQ_SYNTAX);
        return;
      }
      if (arg2 > 255) {
        report_err("%s: Invalid count %d in |@", iname, arg2);
        status_return(SBS_REQ_SYNTAX);
        return;
      }
      if ((arg3&0xFF00) != (arg1&0xFF00)) {
        report_err(
          "%s: Invalid RdAddrNoInc %X|%X@%X to multiple devices.",
          iname, arg1, arg3);
        status_return(SBS_REQ_SYNTAX);
        return;
      }
      can_msg.device_id = (arg1>>8) & 0xFF;
      can_msg.sb_can_cmd = CAN_CMD_CODE_RD_CNT_NOINC;
      can_msg.sb_can_seq = 0;
      can_msg.sb_can[0] = arg1&0xFF;
      can_msg.sb_can[1] = arg2&0xFF;
      can_msg.sb_can[2] = arg3&0xFF;
      can_msg.sb_nb = 3;
      can_msg.end_of_request = (buf[cp] == '\n');
      flavor->enqueue_request(&can_msg,
          (uint8_t*)&rep.data.mread.rvals[rep.data.mread.n_reads],
          2+arg2*2, this);
      break;
    case '\n':
      msg(4, "%s: '\\n' should have been handled", iname);
  }
}

/**
 * On entry, buf[cp] should either point to '\n' or ','
 * I use while(){switch() { }} with both 'continue' and 'break' statements
 * inside the switch. A break inside the switch will hit the break outside
 * the switch and exit the while loop. A continue inside the switch will
 * skip the break and go to the next iteration step.
 */
void subbusd_CAN_client::format_mread_rd() {
  uint16_t arg1, arg2, arg3;
  nl_assert(cp < nc);
  while (cp < nc && buf[cp] == ',') {
    unsigned int cp_sav = cp++;
    if (not_hex(arg1)) {
      if (cp >= nc)
        report_err("%s: Truncated mread?", iname);
      status_return(SBS_REQ_SYNTAX);
      return;
    }
    switch (buf[cp]) {
      case '\n':
      case ',':
        if (((arg1>>8)&0xFF) != can_msg.device_id) {
          cp = cp_sav;
          break;
        }
        can_msg.sb_can[can_msg.sb_nb++] = arg1&0xFF;
        continue;
      case ':':
        ++cp;
        if (not_hex(arg2) || not_str(":") || not_hex(arg3) || cp >= nc) {
          if (cp >= nc)
            report_err("%s: Truncated mread?", iname);
          status_return(SBS_REQ_SYNTAX);
          return;
        }
        if (arg3 < arg1 || (arg3&0xFF00) != (arg1&0xFF00)) {
          report_err("%s: Invalid RD_INC %X:1:%X", iname, arg1, arg3);
          status_return(SBS_REQ_SYNTAX);
          return;
        }
        if (((arg1>>8)&0xFF) != can_msg.device_id || arg2 == 1) {
          cp = cp_sav;
          break;
        }
        // setup CAN_CMD_CODE_RD and unwind
        for ( ; arg1 <= arg3; arg1 += arg2) {
          can_msg.sb_can[can_msg.sb_nb] = arg1 & 0xFF;
          ++can_msg.sb_nb;
        }
        continue;
      default:
        cp = cp_sav;
        break;
    }
    break;
  }
  can_msg.end_of_request = (buf[cp] == '\n');
  flavor->enqueue_request(&can_msg,
    (uint8_t*)&rep.data.mread.rvals[rep.data.mread.n_reads],
    can_msg.sb_nb*2, this);
}

subbusd_CAN::subbusd_CAN() : subbusd_flavor("CAN", new_subbusd_CAN_client) {}
subbusd_CAN::~subbusd_CAN() {}

void subbusd_CAN::init_subbus() {
  // setup socket
  CAN = new CAN_interface();
  CAN->reference();
  subbusd_core::subbusd->srvr.ELoop.add_child(CAN->iface_ptr());
  CAN->setup();
}

void subbusd_CAN::shutdown_subbus() {
  // teardown socket
  if (CAN) {
    CAN->cleanup();
    CAN->dereference();
    CAN = 0;
  }
}

void subbusd_CAN_init_options(int argc, char **argv) {
  argc = argc;
  argv = argv;
  subbusd_CAN *CAN = new subbusd_CAN();
  nl_assert(subbusd_core::subbusd);
  subbusd_core::subbusd->register_flavor(CAN);
}
