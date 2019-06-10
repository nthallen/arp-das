#include "meerstetter_int.h"

Me_Ser::Me_Ser() : Ser_Selectee() {
  pending = 0;
  flags = Fl_Read | gflag(0);
}

void Me_Ser::enqueue_request(Me_Query *req, bool persistent) {
  if (persistent)
    TM_queue.push_back(req);
  else
    Transient_queue.push_back(req);
  process_requests();
}

Me_Query *Me_Ser::new_query() {
  Me_Query *Q;
  if (Free_queue.empty()) {
  } else {
    Q = Free_queue.front();
    Free_queue.pop_front();
  }
  Q->init();
  return Q;
}

bool Me_Ser::ProcessData(int flags) {
  if ((flags & flag & gflag(0)) && tm_sync())
    return true;
  if ((flags&Fl_Read) && (flags&flag&(Fl_Read|Fl_Timeout))) {
    if (fillbuf(bufsize, flag)) return true;
    if (fd < 0) return false;
    if (protocol_input()) return true;
  }
  // if ((flags & flag & Fl_Write) && iwrite_check())
    // return true;
  // if ((flags & flag & Fl_Except) && protocol_except())
    // return true;
  if ((flags & flag & Fl_Timeout) && TO.Expired() && protocol_timeout())
    return true;
  if (TO.Set()) {
    flags |= Fl_Timeout;
  } else {
    flags &= ~Fl_Timeout;
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
  for (i = 0; i < width; ++i) {
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
  if (!pending) {
    report_err("Unexpected input");
    consume(nc);
    return false;
  }
  if (not_found('!')) return;
  if (cp > 1) {
    consume(cp-1);
    cp = 1;
  }
  if (not_hex(address, 2) || not_hex(seq_num, 4)) {
    if (cp >= nc) return false;
    report_err("Invalid response");
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
        re_crc = crc16xmodem_byte(0, &buf[0], cp-5);
        if (crc != re_crc) {
          report_err("Invalid CRC %u on error response, received %lu", re_crc, crc);
        } else {
          report_err("Error code %lu", err_code);
        }
      }
      consume(nc);
      return false;
    }
    if (pending->ret_type != Me_ACK) {
      if (not_hex(value,8)) {
        if (cp < nc)
          consume(nc);
        return false;
      }
      re_crc = pending->req_crc;
    } else {
      re_crc = crc16xmodem_byte(0, &buf[0], cp);
    }
    if (not_hex(crc, 4) || not_str("\r")) {
      if (cp < nc)
        consume(nc);
      return false;
    } else if (crc != re_crc) {
        report_err("Bad CRC: Calculated %lu, expected %u", crc, re_crc);
    } else if (pending->ret_type == Me_INT32) {
      int32_t *src_ptr = (int32_t *)(&value);
      if (pending->ret_ptr) {
        int32_t *dest_ptr = (int32_t*)pending->ret_ptr;
        *dest_ptr = *src_ptr;
      } else {
        msg(0, "Read(%u,%u) = %ld", pending->address, pending->MeParID, *src_ptr);
      }
    } else if (pending->ret_type == Me_FLOAT32) {
      float *src_ptr = (float *)(&value);
      if (pending->ret_ptr) {
        float *dest_ptr = (float*)pending->ret_ptr;
        *dest_ptr = *src_ptr;
      } else {
        msg(0, "Read(%u,%u) = %f", pending->address, pending->MeParID, *src_ptr);
      }
    }
    consume(nc);
  }
  return false;
}

bool Me_Ser::protocol_timeout() {
  TO.clear();
  if (pending) {
    report_err("Timeout on %s address %u MeParID %u",
      pending->MeParType == Me_ACK ? "command to" : "query from",
      pending->address, pending->MeParID);
    if (!pending->persistent) {
      Free_queue.push_back(pending);
    }
    pending = 0;
    process_requests();
  } else {
    report_err("Unexpected timeout while not pending");
  }
  return false;
}

bool tm_sync() {
  if (cur_poll == TM_queue.end()) {
    cur_poll = TM_queue.begin();
    process_requests();
  }
}

void process_requests() {
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
  int cmdlen;
  const char *cmd = pending->get_cmd(&cmdlen);
  int rc = write(fd, cmd, cmdlen);
  if (rc != cmdlen) {
    nl_error(3, "Incomplete write to Meerstetter: %d/%d", rc, cmdlen);
  }
  TO.Set(0, 100);
}
