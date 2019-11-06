#include <stdio.h>
#include "mksflow_int.h"
#include "mksflow.h"
#include "msg.h"
#include "nortlib.h"

/*
 * Cmd client: MKS_Cmd -> Cmd_Selectee
 * TM client: TM_Selectee
 * Serial client: MKS_Ser -> Ser_Selectee
 */

static const char *cmd_name(const char *name) {
  static char nbuf[80];
  int nc = snprintf(nbuf,80,"cmd/%s", name);
  if (nc >= 80)
    msg(MSG_FATAL, "Name length exceeded in cmd_name()");
  return nbuf;
}

MKS_Cmd::MKS_Cmd(MKS_Ser *ser)
    : Cmd_Selectee(cmd_name(MKS_Name), 80),
      ser(ser) {
}

int MKS_Cmd::ProcessData(int flag) {
  // Fill in with code from DAS_IO::Interface::ProcessData()
  // I have commented out code we are not using, since we
  // do not have the default functions referenced
  // if ((flags & flag & gflag(0)) && tm_sync())
    // return true;
  if ((flags&Selector::Sel_Read) && (flags&flag&(Selector::Sel_Read|Selector::Sel_Timeout))) {
    if (fillbuf()) return true;
    if (fd < 0) return false;
    if (app_input()) return true;
  }
  // if ((flags & flag & Sel_Write) && iwrite_check())
    // return true;
  // if ((flags & flag & Sel_Except) && protocol_except())
    // return true;
  // if ((flags & flag & Sel_Timeout) && TO.Expired() && protocol_timeout())
    // return true;
  // if (TO.Set()) {
    // flags |= Sel_Timeout;
  // } else {
    // flags &= ~Sel_Timeout;
  // }
  return false;
}

/**
 * Command formats:
 *  W<address_decimal>:<MKSParID_decimal>[:<float>]
 *  R<address_decimal>:<MKSParID_decimal>
 *  Q
 */
bool MKS_Cmd::app_input() {
  uint8_t address;
  uint16_t MKSParID;
  int index;
  float float_val;
  int wfloat_cp;
  bool has_float = false;
  MKS_Query *Q;
  if (nc == 0) return true;
  if (not_any("RWQ")) {
    consume(nc);
    return false;
  }
  cp = 1;
  char cmdtext[20];
  const char *cmdptr, *capptr;

  switch (buf[0]) {
    case 'Q':
      report_ok();
      consume(nc);
      return true;
    case 'W':
      if (not_uint8(address) || not_str(":") ||
          not_uint16(MKSParID)) {
        report_err("Invalid or incomplete write command");
        break;
      }
      if (cp < nc && buf[cp] == ':') {
        wfloat_cp = ++cp;
        if (not_float(float_val)) {
          report_err("Missing or invalid float in write");
          break;
        }
        if (cp >= nc || buf[cp] != '\n') {
          report_err("Expected newline after float in write");
          break;
        }
        buf[cp] = '\0';
        has_float = true;
      }
      index = get_addr_index(address);
      if (index < 0) {
        report_err("Invalid address");
        break;
      }
      switch (MKSParID) {
        case 1:
          snprintf(cmdtext,20,"SX!%s",&buf[wfloat_cp]);
          cmdptr = cmdtext;
          break;
        case 2:
          snprintf(cmdtext,20,"FT!%s",&buf[wfloat_cp]);
          cmdptr = cmdtext;
          break;
        case 5:
          cmdptr = "SR";
          break;
        default:
          report_err("Unsupported MKSParID: %d", MKSParID);
          break;
      }
      Q = ser->new_query();
      Q->setup_query(address, cmdptr, 0, 0, 0, 0);
      ser->enqueue_request(Q);
      report_ok();
      break;
    case 'R':
      if (not_uint8(address) || not_str(":") ||
          not_uint16(MKSParID) || not_str("\n")) {
        if (cp >= nc)
          report_err("Invalid or incomplete read command");
        break;
      }
      index = get_addr_index(address);
      if (index < 0) {
        report_err("Invalid address");
        break;
      }
      switch (MKSParID) {
        case 2:
          cmdptr = "FT?";
          capptr = board_id[index].gas_units;
          break;
        case 3: cmdptr = "TA?"; capptr = "C"; break;
        case 4: cmdptr = "RH?"; capptr = "Run Hours"; break;
        default:
          report_err("Unsupported MKSParID: %d", MKSParID);
          consume(nc);
          return false;
      }
      Q = ser->new_query();
      Q->setup_query(address, cmdptr, 0, 0, 0, 0);
      Q->set_caption(capptr);
      Q->set_callback(cb_report);
      ser->enqueue_request(Q);
      report_ok();
      break;
    default:
      report_err("Invalid command");
      break;
  }
  consume(nc);
  return false;
}

/**
 * Parsing utility function to read in a hex integer starting
 * at the current position. Integer may be proceeded by optional
 * whitespace.
 * @param[out] hexval The integer value
 * @return zero if an integer was converted, non-zero if the current char is not a digit.
 */
int MKS_Cmd::not_hex(uint32_t &hexval) {
  hexval = 0;
  while (cp < nc && isspace(buf[cp]))
    ++cp;
  if (! isxdigit(buf[cp])) {
    if (cp < nc)
      report_err("No hex digits at col %d", cp);
    return 1;
  }
  while ( cp < nc && isxdigit(buf[cp]) ) {
    unsigned short digval = isdigit(buf[cp]) ? ( buf[cp] - '0' ) :
           ( tolower(buf[cp]) - 'a' + 10 );
    hexval = hexval * 16 + digval;
    ++cp;
  }
  return 0;
}

bool MKS_Cmd::not_any(const char *alternatives) {
  if (cp < nc) {
    for (const char *alt = alternatives; *alt; ++alt) {
      if (buf[cp] == *alt) {
        ++cp;
        return false;
      }
    }
    report_err("No match for alternatives '%s' at column %d", alternatives, cp);
  }
  return true;
}

bool MKS_Cmd::not_uint16(uint16_t &output_val) {
  uint32_t val = 0;
  if (buf[cp] == '-') {
    if (isdigit(buf[++cp])) {
      while (isdigit(buf[cp])) {
        ++cp;
      }
      if (cp < nc)
        report_err("not_uint16: Negative int truncated at col %d",
          cp);
    } else {
      if (cp < nc)
        report_err("Found '-' and no digits at col %d", cp);
      return true;
    }
  } else if (isdigit(buf[cp])) {
    while (isdigit(buf[cp])) {
      val = val*10 + buf[cp++] - '0';
    }
  } else {
    if (cp < nc)
      report_err("not_uint16: no digits at col %d", cp);
    return true;
  }
  if (val > 65535) {
    report_err("value exceeds uint16_t range at col %d", cp--);
    return true;
  }
  output_val = val;
  return false;
}

bool MKS_Cmd::not_uint8(uint8_t &val) {
  uint16_t sval;
  if (not_uint16(sval)) return true;
  if (sval > 255) {
    report_err("uint8_t value out of range: %u at col %d",
      sval, cp--);
    return true;
  }
  val = sval;
  return false;
}
