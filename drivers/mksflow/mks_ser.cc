#include <fcntl.h>
#include "mksflow_int.h"
#include "nortlib.h"
#include "msg.h"
#include "nl_assert.h"

bool rs485_echos = false;

MKS_Ser::MKS_Ser(const char *path) : Ser_Sel(path, O_RDWR|O_NONBLOCK, 400) {
  pending = 0;
  all_polling = false;
  retry_delay = 0;
  cur_poll = TM_queue.end();
  flags = Selector::Sel_Read | Selector::gflag(0);
}

void MKS_Ser::enqueue_request(MKS_Query *req) {
  if (req->persistent)
    TM_queue.push_back(req);
  else
    Transient_queue.push_back(req);
  req->ser = this;
  process_requests();
}

MKS_Query *MKS_Ser::new_query() {
  MKS_Query *Q;
  if (Free_queue.empty()) {
    Q = new MKS_Query();
  } else {
    Q = Free_queue.front();
    Free_queue.pop_front();
  }
  Q->init();
  return Q;
}

bool MKS_Ser::checksum_verify(int from, int to, int checksum) {
  int recalc = 0;
  for (int i = from; i < to; ++i) {
    recalc += buf[i];
  }
  recalc &= 0xFF;
  if (recalc != checksum) {
    board_id_t *bdp = &board_id[pending->get_index()];
    report_err("%s: checksum error", bdp->mnemonic);
    return false;
  }
  return true;
}

int MKS_Ser::ProcessData(int flag) {
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
int MKS_Ser::not_hex(uint32_t &hexval, int width) {
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

bool MKS_Ser::saw_error() {
  // msg(MSG_DBG(0), "saw_error(): input is '%s'", ascii_escape((const char *)buf));
  if (cp >= nc) {
    update_tc_vmin(pending_replen - nc);
  } else {
    consume(nc);
    free_pending();
  }
  return false;
}

bool MKS_Ser::protocol_input() {
  int err_code;
  uint32_t checksum;
  cp = 0;
  if (!pending) {
    report_err("Unexpected input");
    consume(nc);
    return false;
  }
  if (rs485_echos && not_str(pending_cmd, pending_cmdlen))
    return saw_error();
  unsigned int rep_cp = cp;
  if (not_found('@')) {
    // free_pending();
    msg(MSG_DBG(0),"No @ in protocol_input");
    return false;
  }
  nl_assert(cp > 0);
  rep_cp = cp - 1;
  if (not_str("@@000") || cp >= nc)
    return saw_error();
  // if (cp >= nc) {
  //   update_tc_vmin(pending_replen - nc);
  //   return false;
  // }
  if (buf[cp] != 'A') {
    if (not_str("NAK") ||
        not_int(err_code) ||
        not_str(";") ||
        not_hex(checksum,2)) {
      return saw_error();
    }
    checksum_verify(rep_cp, cp-2, checksum);
    report_err("%s: NAK code %d received",
      board_id[pending->get_index()].mnemonic,
      err_code);
    consume(nc);
    free_pending();
  }
  if (not_str("ACK"))
    return saw_error();
  int rep_start = cp;
  while (cp < nc && buf[cp] != ';') ++cp;
  int rep_end = cp;
  if (cp < nc) ++cp;
  if (cp >= nc || not_hex(checksum,2))
    return saw_error();
  checksum_verify(rep_cp, cp-2, checksum);
  
  // Now do something with the results
  pending->clear_bit();
  buf[rep_end] = '\0';
  if (pending->callback)
    (*pending->callback)(pending,
      (const char *)&buf[rep_start]);
  else pending->store_string(0,
          (const char *)&buf[rep_start]);
  consume(nc);
  free_pending();
  return false;
}

bool MKS_Ser::protocol_timeout() {
  TO.Clear();
  // msg(0, "protocol_timeout()");
  if (pending) {
    report_err("%s: Timeout on address %u nc:%d",
      board_id[pending->get_index()].mnemonic,
      pending->address, nc);
    consume(nc);
    free_pending();
  } else {
    report_err("Unexpected timeout while not pending");
    consume(nc);
  }
  return false;
}

bool MKS_Ser::tm_sync() {
  if (!all_polling && --retry_delay <= 0) {
    all_polling = true;
    for (index = 0; index < n_drives; ++index) {
      if (!board_id[index].is_polling) {
        all_polling = false;
        identify_board(ser, index, board_id[index].device_address);
      }
    }
    if (!all_polling) retry_delay = 4;
  }
  if (cur_poll == TM_queue.end()) {
    cur_poll = TM_queue.begin();
  }
  process_requests();
  return false;
}

void MKS_Ser::free_pending() {
  TO.Clear();
  if (pending) {
    if (!pending->persistent) {
      Free_queue.push_back(pending);
    }
    pending = 0;
  }
  process_requests();
}

void MKS_Ser::process_requests() {
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
  int rc = write(fd, pending_cmd, pending_cmdlen);
  if (rc != pending_cmdlen) {
    nl_error(3, "Incomplete write to mksflow: %d/%d", rc, pending_cmdlen);
  }
  pending_replen = pending->replen + (rs485_echos ? pending_cmdlen : 0);
  update_tc_vmin(pending_replen - nc);
  TO.Set(0, 100);
  flags |= Selector::Sel_Timeout;
}

