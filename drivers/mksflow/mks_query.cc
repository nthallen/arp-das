#include "mksflow_int.h"
#include "nl_assert.h"
#include "msg.h"

MKS_Query::MKS_Query()
  :
    persistent(false),
    ret_ptr(0),
    ret_len(0),
    address(0),
    cmdlen(0),
    callback(0),
    mask_ptr(0),
    mask_bit(0),
    caption(""),
    rep_len(0)
    {}

MKS_Query::~MKS_Query() {}

void MKS_Query::init() {
  cmdlen = 0;
  cmd[0] = '\0';
  persistent = false;
  callback = 0;
  ret_ptr = 0;
  ret_len = 0;
  mask_ptr = 0;
  mask_bit = 0;
  ser = 0;
  address = 0;
  caption = "";
  rep_len = 0;
}

const char *MKS_Query::get_cmd(int *cmdlenptr) {
  SeqNr = ++Sequence_Number;
  to_hex(SeqNr, 4, 3);
  if (crc_applied) {
    cmdlen -= 5;
    crc_applied = false;
  }
  req_crc = crc16xmodem_byte(0, &cmd[0], cmdlen);
  to_hex(req_crc, 4, cmdlen);
  cmd[cmdlen++] = '\r';
  cmd[cmdlen] = '\0';
  crc_applied = true;
  if (cmdlenptr)
    *cmdlenptr = cmdlen;
  return &cmd[0];
}

const char *MKS_Query::get_raw_cmd() {
  return &cmd[0];
}

void MKS_Query::setup_query(uint8_t address, const char *req, char *dest, int dsize, uint8_t *ack, uint8_t bit) {
  cmdlen = snprintf(cmd, max_command_length, "@%03d%s;", address, req);
  uint16_t csum = 0;
  for (int i = 0; i < cmdlen; ++i)
    csum += cmd[i];
  cmdlen += snprintf(cmd+cmdlen, max_command_length-cmdlen, "%02X", csum & 0xFF);
  this->ret_ptr = dest;
  this->ret_len = dsize;
  this->mask_ptr = ack;
  this->mask_bit = bit;
  replen = 12; // @@@000NAKnn;hh @@@000ACK;hh
  this->address = address;
  this->index = get_addr_index(address);
}

void MKS_Query::set_persistent(bool persistent) {
  this->persistent = persistent;
}

void MKS_Query::set_callback(void (*CB)(MKS_Query *, const char *)) {
  callback = CB;
}

// void MKS_Query::setup_address(uint8_t address, uint16_t MKSParID) {
  // this->address = address;
  // this->MKSParID = MKSParID;
  // cmdlen = 0;
  // cmd[0] = '#';
  // to_hex(address, 2, 1);
// }

// void MKS_Query::to_hex(uint32_t val, int width, int cp) {
  // uint32_t rval = val;
  // nl_assert(cp+width < max_command_length);
  // for (int i = width-1; i >= 0; --i) {
    // uint8_t byte = val & 0xF;
    // byte += (byte < 10) ? '0' : ('A'-10);
    // cmd[cp+i] = byte;
    // val >>= 4;
  // }
  // if (val != 0)
    // msg(MSG_WARN, "MKS_Query::to_hex(%lu, %d, %d) has insufficient width", rval, width, cp);
  // if (cp+width > cmdlen)
    // cmdlen = cp+width;
// }

void MKS_Query::set_bit() {
  if (mask_ptr) {
    *mask_ptr |= mask_bit;
  }
}

void MKS_Query::clear_bit() {
  if (mask_ptr) {
    *mask_ptr &= ~mask_bit;
    if ((*mask_ptr & ~1) == 0) {
      *mask_ptr = 0; // clear 0 bit if all others are clear
    }
  }
}

// uint16_t MKS_Query::Sequence_Number = 0;
