#include <stdlib.h>
#include <unistd.h>
#include "subbus.h"
#include "nortlib.h"

static const char *Inst[] = {
  "Bench Machine",
  "Harvard Water Vapor",
  "Harvard Total Water",
  "Harvard Carbon Isotopes" };
#define N_INST_IDS 4

int main(int argc, char **argv) {
  int rv, i;
  unsigned short board_base = 0x1000;
  unsigned short DACSbuild;
  unsigned short inst_id;

  if (load_subbus() ==0)
    nl_error(3, "Unable to load subbus library");
  rv = read_ack( 0, &DACSbuild );
  if ( rv ) nl_error(2, "Unexpected ACK reading from 0" );
  rv = read_ack( 0x80, &DACSbuild );
  if ( rv ) {
    rv = read_ack(0x81, &inst_id);
    if ( !rv )
      nl_error(2, "No ack reading instrument ID for build #%u", DACSbuild);
    else {
      if (inst_id >= N_INST_IDS)
        nl_error( 2, "Instrument ID %u out of range", inst_id);
      else
        nl_error( 0, "%s: DACS Build #%u\n", Inst[inst_id], DACSbuild );
    }
  }
  for (i = 1; i < argc; ++i) {
    unsigned long block_addr = strtoul(argv[i], 0, 16);
    unsigned short short_addr;
    unsigned short ctrlr_status;
    unsigned short remaining;
    unsigned short data[128];
    int j, k;
    if (block_addr > 0xFFFFL)
      nl_error(3, "Block address out of range");
    short_addr = block_addr;
    printf("Reading block 0x%04X\n", short_addr);
    ctrlr_status = sbrwa(board_base);
    if (ctrlr_status & 0x7FF) {
      nl_error(1, "Controller status before verify: 0x%04X, resetting",
        ctrlr_status);
      sbwr(board_base+0xC, 0);
      delay(10);
      ctrlr_status = sbrwa(board_base);
      if (ctrlr_status & 0x7FF)
        nl_error(3, "Did not clear");
    }
    sbwr(board_base+0xA, short_addr);
    for (;;) {
      ctrlr_status = sbrwa(board_base);
      if (ctrlr_status & 0x200) break;
      if (!(ctrlr_status & 0x100))
        nl_error(3, "Controller not in read mode: %04X", ctrlr_status);
    }
    if ((ctrlr_status & 0xB00) != 0xB00)
      nl_error(2, "Expected 0xB00 after verify: 0x%04X", ctrlr_status);
    remaining = ctrlr_status & 0xFF;
    if (remaining != 128)
      nl_error(2, "Expected 128 bytes in FIFO after verify");
    j = 0;
    while (remaining > 0) {
      data[j++] = sbrwa(board_base+4);
      ctrlr_status = sbrwa(board_base);
      if ((ctrlr_status & 0xFF) != --remaining)
        nl_error(1, "Expected remaining count of 0x%02X, read 0x%04X",
          remaining, ctrlr_status);
    }
    printf("  ");
    for (j = 0; j < 128; j += 16)
      printf(" %04X", j);
    printf("\n");
    for (k = 0; k < 16; ++k) {
      printf("%01X:", k);
      for (j = 0; j < 128; j += 16)
        printf(" %04X", data[j+k]);
      printf("\n");
    }
  }
  return 0;
}

