/** @file test_hal_bmm.cc
 * @brief Test interfaces to subbus modules
 */
#include <stdio.h>
#include <strings.h>
#include "oui.h"
#include "subbuspp.h"
#include "nortlib.h"
#include "msg.h"
#include "test_hal_bmm.h"

// DAS_IO::AppID_t DAS_IO::AppID("test_bmm", "BMM Test Program", "V1.0");

uint16_t power_cmd = 0;

typedef struct {
  uint16_t n_words;
  char name[0x50];
} device_name_t;

/* Conversion factors for output in Volts and Amps */
#define VCONV (0.025/16)
#define ICONV28 (20e-3/(16*7))
#define ICONV50 (20e-3/(16*3))
#define ADINCONV28 ((0.5e-3 * (2+29.4))/(16*2))
#define ADINCONV50 ((0.5e-3 * (2+59.0))/(16*2))

void identify_board(subbuspp *P, uint8_t bdid) {
  uint16_t bdid_hi = bdid<<8;
  msg(0,"read_ack(0x%02X02)", bdid);
  uint16_t value;
  if (! P->read_ack(bdid_hi | 0x02, &value)) {
    msg(2, "No acknowledge from board %d", bdid);
    return;
  }
  msg(0, "  Board Class: %u", value);
  value = P->read_subbus(bdid_hi | 0x04);
  msg(0, "  Board S/N:  %u", value);
  value = P->read_subbus(bdid_hi | 0x03);
  msg(0, "  Build No:   %u", value);
  value = P->read_subbus(bdid_hi | 0x05);
  msg(0, "  Inst ID:  %u", value);
  
  char mreqstr[30];
  snprintf(mreqstr, 30, "%X|28@%X", bdid_hi|8, bdid_hi|9);
  subbus_mread_req *mreq = P->pack_mread_request(0x29, mreqstr);
  device_name_t devname;
  uint16_t nread;
  int rv = P->mread_subbus_nw(mreq, (uint16_t*)&devname, &nread);
  if (rv < 0) {
    msg(2, "Error %d from mread", rv);
  } else {
    msg(0, "nr:%u/%u '%s'", nread, devname.n_words, &devname.name[0]);
  }
}

void test_mread(subbuspp *P, uint8_t bdid) {
  uint16_t bdid_hi = bdid<<8;
  char mreqstr[30];
  snprintf(mreqstr, 30, "%X:1:%X,%X",
    bdid_hi|0x21, bdid_hi|0x28, bdid_hi|0x30);
  subbus_mread_req *mreq = P->pack_mread_request(9, mreqstr);
  uint16_t rval[9];
  uint16_t nread;
  int rv = P->mread_subbus_nw(mreq, rval, &nread);
  if (rv < 0) {
    msg(2, "Error %d from mread", rv);
  } else {
    msg(0, "mr:%u", nread);
    msg(0, "  V1: %5.2lf V", rval[1]*VCONV);
    msg(0, "  V2: %5.2lf V", rval[2]*ADINCONV28);
    msg(0, "   I: %6.3lf A", rval[0]*ICONV50);
    msg(0, "  NR: %u, %u", rval[3], rval[7]);
    msg(0, "PMSt: %04X", rval[4]);
    msg(0, "Cmds: %04X", rval[8]);
    msg(0, "TRU Power reportedly %s via mread",
      (rval[8] & 4) ? "OFF" : "ON");
  }
}

void test_ack(subbuspp *P, uint16_t addr) {
  // msg(0, "Test read from non-existant address on existing board");
  uint16_t value;
  if (! P->read_ack(addr, &value)) {
    msg(2, "No acknowledge from address 0x%04X", addr);
  } else {
    msg(0, "ACK from addr 0x%04X", addr);
  }
}

void test_nack(subbuspp *P, uint16_t addr) {
  // msg(0, "Test read from non-existant address on existing board");
  uint16_t value;
  if (! P->read_ack(addr, &value)) {
    msg(0, "No acknowledge from address 0x%04X", addr);
  } else {
    msg(2, "Unexpected ACK from addr 0x%04X", addr);
  }
}

int main(int argc, char **argv) {
  oui_init_options(argc, argv);
  subbuspp *P = new subbuspp("/dev/huarp/subbus");
  int subfunc = P->load();
  if (subfunc) {
    msg(0, "Subbus subfunction %d, name %s",
        subfunc, P->get_subbus_name());
  } else {
    msg(3, "Failed to connect with subbus");
  }

  identify_board(P, 1);
  uint16_t cmdstat = P->read_subbus(0x0130);
  msg(0, "TRU Power originally %s",
    (cmdstat & 4) ? "OFF" : "ON");

  uint16_t PM0I1 = P->read_subbus(0x0121);
  uint16_t PM0V1 = P->read_subbus(0x0122);
  uint16_t PM0V2 = P->read_subbus(0x0123);
  msg(0, "  V1: %5.2lf V", PM0V1*VCONV);
  msg(0, "  V2: %5.2lf V", PM0V2*ADINCONV28);
  msg(0, "   I: %6.3lf A", PM0I1*ICONV50);

  test_mread(P, 1);

  if (power_cmd) {
    msg(0, "Issuing command %d", power_cmd);
    if (!P->write_ack(0x0130, power_cmd)) {
      msg(2, "No ACK writing to 0x0130");
    }
    cmdstat = P->read_subbus(0x0130);
    msg(0, "TRU Power now %s",
      (cmdstat & 4) ? "OFF" : "ON");
  }

  // msg(0, "Test read from non-existant board");
  // identify_board(P, 2);
  
  // test_ack(P, 0x0121);
  // test_nack(P, 0x0140);
  // test_ack(P, 0x0121);
  // test_nack(P, 0x0140);

  // P->subbus_quit();
  return 0;
}
