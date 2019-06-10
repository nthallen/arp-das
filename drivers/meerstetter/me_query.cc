#include "meerstetter_int.h"
#include "crc16xmodem.h"

Me_Query::Me_Query()
  : cmdlen(0),
    persistent(false),
    ret_type(Me_ACK),
    ret_ptr(0),
    address(0),
    MeParID(0),
    SeqNr(0)
    {}

Me_Query::~Me_Query() {}

void Me_Query::init() {
  cmdlen = 0;
  persistent = false;
  ret_type = Me_ACK;
  ret_ptr = 0;
  address = 0;
  MeParID = 0;
  SeqNr = 0;
}

const char *Me_Query::get_cmd(int *cmdlenptr) {
  SeqNr = ++Sequence_Number;
  to_hex(SeqNr, 4, 3);
  req_crc = crc16xmodem_byte(0, &cmd[0], cmdlen);
  to_hex(req_crc, 4, cmdlen);
  cmd[cmdlen++] = '\r';
  cmd[cmdlen] = '\0';
  if (cmdlenptr)
    *cmdlenptr = cmdlen;
  return &cmd[0];
}

const char *Me_Query::get_raw_cmd() {
  return &cmd[0];
}

void Me_Query::setup_int32_query(uint8_t address, uint16_t MeParID, int32_t *ret_ptr) {
  setup_address(address, MeParID);
  cmd[7] = '?'; cmd[8] = 'V'; cmd[9] = 'R';
  to_hex(MeParID, 4, 10);
  to_hex(1, 2, cmdlen); //instance == 1
  if (ret_ptr) {
    ret_type = Me_INT32;
    this->ret_ptr = (void *)ret_ptr;
    set_persistent(true);
  } else {
    set_persistent(false);
  }
}

void Me_Query::setup_float32_query(uint8_t address, uint16_t MeParID, float *ret_ptr) {
  setup_address(address, MeParID);
  cmd[7] = '?'; cmd[8] = 'V'; cmd[9] = 'R';
  to_hex(MeParID, 4, 10);
  to_hex(1, 2, cmdlen); //instance == 1
  if (ret_ptr) {
    ret_type = Me_FLOAT32;
    this->ret_ptr = (void *)ret_ptr;
    set_persistent(true);
  } else {
    set_persistent(false);
  }
}

void Me_Query::setup_uint32_cmd(uint8_t address, uint16_t MeParID, uint32_t value) {
  setup_address(address, MeParID);
  cmd[7] = 'V';
  cmd[8] = 'S';
  to_hex(MeParID, 4, 9);
  to_hex(1, 2, cmdlen); //instance == 1
  to_hex(value, 8, cmdlen);
  ret_type = Me_ACK;
}

void Me_Query::set_persistent(bool persistent) {
  this->persistent = persistent;
}

void Me_Query::setup_address(uint8_t address, uint16_t MeParID) {
  this->address = address;
  this->MeParID = MeParID;
  cmdlen = 0;
  cmd[0] = '#';
  to_hex(address, 2, 1);
}

void Me_Query::to_hex(uint32_t val, int width, int cp) {
  uint32_t rval = val;
  nl_assert(cp+width < max_command_length);
  for (int i = width-1; i >= 0; --i) {
    uint8_t byte = val & 0xFF;
    byte += (byte <= 10) ? '0' : ('A'-10);
    cmd[cp+i] = byte;
    val >>= 8;
  }
  if (val != 0)
    msg(MSG_WARN, "Me_Query::to_hex(%lu, %d, %d) has insufficient width", rval, width, cp);
  if (cp+width > cmdlen)
    cmdlen = cp+width;
}

uint16_t Me_Query::Sequence_Number = 0;
