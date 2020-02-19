#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "meerstetter_int.h"
#include "crc16xmodem.h"
#include "nortlib.h"
#include "msg.h"

bool rs485_echos = false;

Me_Ser::Me_Ser(const char *path) : Ser_Sel(path, O_RDWR|O_NONBLOCK, 400) {
  pending = 0;
  cur_poll = TM_queue.end();
  flags = Selector::Sel_Read | Selector::gflag(0);
}

void Me_Ser::enqueue_request(Me_Query *req) {
  if (req->persistent)
    TM_queue.push_back(req);
  else
    Transient_queue.push_back(req);
  process_requests();
}

Me_Query *Me_Ser::new_query() {
  Me_Query *Q;
  if (Free_queue.empty()) {
    Q = new Me_Query();
  } else {
    Q = Free_queue.front();
    Free_queue.pop_front();
  }
  Q->init();
  return Q;
}

int Me_Ser::ProcessData(int flag) {
  msg(MSG_DBG(1),
    "ProcessData: flags 0x%2X flag: 0x%2X TO: %s",
    flags, flag,
    TO.Set() ? (TO.Expired() ? "Expired" : "Set") : "Clear");
  if ((flags & flag & Selector::gflag(0)) && tm_sync())
    return true;
  if ((flags&Selector::Sel_Read) &&
      (flags&flag&(Selector::Sel_Read|Selector::Sel_Timeout))) {
    if (fillbuf()) return true;
    if (fd < 0) return false;
    if (protocol_input()) return true;
  }
  // if ((flags & flag & Fl_Write) && iwrite_check())
    // return true;
  // if ((flags & flag & Fl_Except) && protocol_except())
    // return true;
  if ((flags & flag & Selector::Sel_Timeout) &&
        TO.Expired() && protocol_timeout())
    return true;
  if (TO.Set()) {
    flags |= Selector::Sel_Timeout;
  } else {
    flags &= ~Selector::Sel_Timeout;
  }
  return false;
}

/**
 * Parsing utility function to read in a hex integer starting
 * at the current position. Integer may be proceeded by optional
 * whitespace.
 * @param[out] hexval The integer value
 * @param width The number of characters to parse
 * @return zero if an integer was converted, non-zero if the current char is not a digit.
 */
int Me_Ser::not_hex(uint32_t &hexval, int width) {
  hexval = 0;
  for (int i = 0; i < width; ++i) {
    if (cp >= nc) return 1;
    if (!isxdigit(buf[cp])) {
      report_err("Expected hex digit at col %d", cp);
      return 1;
    }
    unsigned short digval =
      isdigit(buf[cp]) ? ( buf[cp] - '0' ) :
           ( tolower(buf[cp]) - 'a' + 10 );
    hexval = hexval * 16 + digval;
    ++cp;
  }
  return 0;
}

bool Me_Ser::protocol_input() {
  uint32_t address, seq_num, value, crc, err_code;
  uint16_t re_crc;
  cp = 0;
  if (!pending) {
    report_err("Unexpected input");
    consume(nc);
    return false;
  }
  if (rs485_echos && not_str(pending_cmd, pending_cmdlen)) {
    if (cp >= nc) {
      update_tc_vmin(pending_replen - nc);
    } else {
      consume(nc);
      free_pending();
    }
    return false;
  }
  unsigned int rep_cp = cp;
  if (not_str("!")) {
    if (cp >= nc) {
      update_tc_vmin(pending_replen - nc);
    } else {
      consume(nc);
      free_pending();
    }
    return false;
  }
  if (not_hex(address, 2) || not_hex(seq_num, 4)) {
    if (cp >= nc) {
      update_tc_vmin(pending_replen - nc);
    } else {
      consume(nc);
      free_pending();
    }
    return false;
  }
  if (address != pending->address || seq_num != pending->SeqNr)
    report_err("Response (addr,SeqNr) (%lu,%lu) != Request (%u,%u)",
      address, seq_num, pending->address, pending->SeqNr);
  // Now look at payload
  // +XX Error packets
  // XXXXXXXX query response
  // ACK has an empty payload and the CRC from the sent command,
  //   *not* the CRC from this packet
  if (cp < nc) {
    if (buf[cp] == '+') {
      ++cp;
      if (not_hex(err_code, 2) || not_hex(crc, 4) || not_str("\r")) {
        if (cp >= nc) return false;
      } else {
        re_crc = crc16xmodem_byte(0, &buf[rep_cp], cp-rep_cp-5);
        if (crc != re_crc) {
          report_err("Invalid CRC %u on error response, received %lu", re_crc, crc);
        } else {
          report_err("Error code %lu", err_code);
        }
      }
      consume(nc);
      free_pending();
      return false;
    }
    if (pending->ret_type != Me_Query::Me_ACK) {
      if (not_hex(value,8)) {
        if (cp >= nc) {
          update_tc_vmin(pending_replen-nc);
        } else {
          consume(nc);
          free_pending();
        }
        return false;
      }
      re_crc = crc16xmodem_byte(0, &buf[rep_cp], cp-rep_cp);
    } else {
      re_crc = pending->req_crc;
    }
    if (not_hex(crc, 4) || not_str("\r")) {
      if (cp >= nc) {
        update_tc_vmin(pending_replen-nc);
      } else {
        consume(nc);
        free_pending();
      }
      return false;
    } else if (crc != re_crc) {
      report_err("Bad CRC: Calculated %lu, expected %u", crc, re_crc);
      consume(nc);
      free_pending();
      return false;
    } else if (pending->ret_type == Me_Query::Me_INT32) {
      int32_t *src_ptr = (int32_t *)(&value);
      if (pending->ret_ptr) {
        int32_t *dest_ptr = (int32_t*)pending->ret_ptr;
        *dest_ptr = *src_ptr;
      } else {
        msg(0, "Read(%u,%u) = %ld", pending->address, pending->MeParID, *src_ptr);
      }
      pending->clear_bit();
      if (pending->callback)
        (*pending->callback)(pending);
    } else if (pending->ret_type == Me_Query::Me_FLOAT32) {
      float *src_ptr = (float *)(&value);
      if (pending->ret_ptr) {
        float *dest_ptr = (float*)pending->ret_ptr;
        *dest_ptr = *src_ptr;
      } else {
        msg(0, "Read(%u,%u) = %f", pending->address, pending->MeParID, *src_ptr);
      }
      pending->clear_bit();
      if (pending->callback)
        (*pending->callback)(pending);
    }
    consume(nc);
    free_pending();
  }
  return false;
}

bool Me_Ser::protocol_timeout() {
  TO.Clear();
  if (pending) {
    report_err("Timeout on %s address %u MeParID %u nc:%d",
      pending->ret_type == Me_Query::Me_ACK ? "command to" : "query from",
      pending->address, pending->MeParID, nc);
    consume(nc);
    free_pending();
  } else {
    report_err("Unexpected timeout while not pending");
    consume(nc);
  }
  return false;
}

bool Me_Ser::tm_sync() {
  if (cur_poll == TM_queue.end()) {
    cur_poll = TM_queue.begin();
    process_requests();
  }
  return false;
}

void Me_Ser::free_pending() {
  TO.Clear();
  if (pending) {
    if (!pending->persistent) {
      Free_queue.push_back(pending);
    }
    pending = 0;
  }
  process_requests();
}

void Me_Ser::process_requests() {
  if (pending) return;
  if (!Transient_queue.empty()) {
    pending = Transient_queue.front();
    Transient_queue.pop_front();
  } else if (cur_poll != TM_queue.end()) {
    pending = *cur_poll;
    ++cur_poll;
  } else {
    return;
  }
  pending_cmd = pending->get_cmd(&pending_cmdlen);
  msg(MSG_DBG(0), "Write Req: '%s'", ascii_escape(pending_cmd));
  pending->set_bit();
  // set_RTS(true);
  int rc = write(fd, pending_cmd, pending_cmdlen);
  if (rc != pending_cmdlen) {
    nl_error(3, "Incomplete write to Meerstetter: %d/%d", rc, pending_cmdlen);
  }
  // if (tcdrain(fd) < 0)
  //   report_err("tcdrain() returned error %d", errno);
  // set_RTS(false);
  pending_replen = pending->replen + (rs485_echos ? pending_cmdlen : 0);
  update_tc_vmin(pending_replen - nc);
  TO.Set(0, 100);
  flags |= Selector::Sel_Timeout;
}

void Me_Ser::set_RTS(bool RTS) {
    int error;
    int data = _CTL_RTS_CHG | (RTS ? _CTL_RTS : 0);

    if ((error = devctl (fd, DCMD_CHR_SERCTL, &data,
                   sizeof(data), NULL))) {
      report_err("Error setting RTS: %s", strerror(error));
    }
}
