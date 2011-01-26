#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "nortlib.h"
#include "subbus.h"

#define IDXR_TEST 1

unsigned short read_report( unsigned short addr, char *desc ) {
  unsigned short data;
  int rv;
  rv = read_ack(addr, &data);
  if ( !rv ) nl_error(2, "No ACK from 0x%04X", addr );
  nl_error( 0, "  %04X(%s): %04X", addr, desc, data);
  return data;
}

static void indexer_test(void) {
  tick_sic();
  // set_failure(0);
  read_report(0x0A00, "Bd Status");
  read_report(0x0A0C, "Position ");
  read_report(0x0A0E, "Ch Status");
  set_cmdenbl(1);
  // set_failure(2);
  nl_error(0, "Set configuration (16KHz, rev limit polarity), clear position" );
  write_ack( 0x0A0C, 0x0000 );
  write_ack( 0x0A0E, 0xFE0 ); // Set config: 16KHz, reverse limit polarity

  nl_error(0, "CmdEnbl set" );
  read_report(0x0A0C, "Position ");
  read_report(0x0A0E, "Ch Status");

  write_ack( 0x0A08, 0 ); // drive in 0
  nl_error(0, "Drive in 0");
  read_report(0x0A0C, "Position ");
  read_report(0x0A0E, "Ch Status");
  sleep(1);

  // set_failure(4);
  tick_sic();
  write_ack( 0x0A0A, 0 ); // drive out 0
  nl_error(0, "Drive out 0");
  read_report(0x0A0C, "Position ");
  read_report(0x0A0E, "Ch Status");
  sleep(1);

  // set_failure(8);
  tick_sic();
  nl_error(0, "Drive out 0x5555");
  write_ack( 0x0A0A, 0x5555 ); // drive out 0
  sleep(1);

  tick_sic();
  read_report(0x0A00, "Bd Status");
  read_report(0x0A0C, "Position ");
  read_report(0x0A0E, "Ch Status");
  nl_error(0, "Wait for drive to complete");
  sleep(1); // 0x5555 ~= 21845

  // set_failure(0xE);
  tick_sic();
  read_report(0x0A00, "Bd Status");
  read_report(0x0A0C, "Position ");
  read_report(0x0A0E, "Ch Status");
  sleep(1);

  // set_failure(0);
  tick_sic();
  nl_error(0, "Just set the step number" );
  write_ack( 0x0A0C, 0x5555 );
  read_report(0x0A0C, "Position ");
  set_cmdenbl(0);
  disarm_sic();
}

static void timeout_test(void) {
  set_cmdenbl(0);
  set_cmdenbl(1);
  // set_failure(8);
  tick_sic();
  sleep(1);

  // set_cmdenbl(1);
  // set_failure(4);
  tick_sic();
  sleep(1);

  // set_cmdenbl(1);
  // set_failure(2);
  tick_sic();
  sleep(1);

  set_failure(0);
}

/* return non-zero on error */
static int ana_in_allcfg( unsigned short wcfg ) {
  unsigned short addr;
  int err = 0;

  for ( addr = 0xC00; addr < 0xD00; addr += 2) {
    sbwr( addr, wcfg );
  }
  usleep( 220 );
  for ( addr = 0xC00; addr < 0xD00; addr += 2) {
    unsigned short data, cfg;
    data = sbrwa(addr);
    cfg = sbrwa(addr+1);
    if ( cfg != wcfg ) {
      nl_error( 2, "Config readback from %03X was %04X, expected %04X",
	addr+1, cfg, wcfg);
      err = 1;
    }
  }
  return err;
}

static void ana_in_cfg(void) {
  unsigned short addr;
  int ok = 1;

  nl_error(0, "Running ana_in config tests" );
  if ( ana_in_allcfg( 0 ) ) ok = 0;
  if ( ana_in_allcfg( 0x10 ) ) ok = 0;
  if ( ana_in_allcfg( 0x0 ) ) ok = 0;
  nl_error(0, "Testing individual channels" );
  for ( addr = 0xC00; addr < 0xD00; addr += 2) {
    unsigned short addr2;
    sbwr( addr, 0x10 );
    usleep(220);
    for ( addr2 = 0xC00; addr2 < 0xD00; addr2 += 2) {
      unsigned short data, cfg;
      unsigned short exp = (addr2 == addr) ? 0x10 : 0;
      data = sbrwa(addr2);
      cfg = sbrwa(addr2+1);
      if ( cfg != exp ) {
	nl_error( 2, "CONFIG readback from %03X was %04X, expected %04X",
	  addr2+1, cfg, exp );
	ok = 0;
      }
    }
    sbwr( addr, 0 );
  }
  if ( ok )
    nl_error( 0, "All ana_in config tests passed." );
}

static void ana_in_mux(void) {
  int ok = 1;
  int i;
  unsigned short addr;

  nl_error(0, "Running ana_in mux config tests" );
  // Start by setting all the standard cfgs to zero
  if ( ana_in_allcfg( 0 ) ) ok = 0;
  for ( i = 0; i < 8; ++i ) {
    sbwr(0xD10 + 2*i, 0);
  }
  sbwr(0xC1E, 0x100);
  usleep(220*8);
  // read the cfgs and verify all 0x100
  for (addr = 0xD10; addr <= 0xD1E; addr += 2 ) {
    unsigned short data, cfg;
    data = sbrwa(addr);
    cfg = sbrwa(addr+1);
    if ( cfg != 0x100 ) {
      nl_error( 2, "Initial CONFIG readback from %03X was %04X, expected %04X",
	addr+1, cfg, 0x100 );
      ok = 0;
    }
  }
  // for each sub, set gain.
  for (addr = 0xD10; addr <= 0xD1E; addr += 2 ) {
    unsigned short addr2;
    sbwr(addr, 0x14);
    usleep(220*8);
    for (addr2 = 0xD10; addr2 <= 0xD1E; addr2 += 2 ) {
      unsigned short data, cfg;
      unsigned short exp = (addr2 == addr) ? 0x114 : 0x100;
      data = sbrwa(addr2);
      cfg = sbrwa(addr2+1);
      if ( cfg != exp ) {
	nl_error( 2, "CONFIG readback from %03X was %04X, expected %04X",
	  addr2+1, cfg, exp );
	ok = 0;
      }
    }
    sbwr(addr, 0);
  }
  if ( ok )
    nl_error( 0, "All ana_in mux tests passed." );
}

int main(int argc, char **argv) {
  int rv, i;
  unsigned short data;

  if ( load_subbus() == 0 )
    nl_error( 3, "Unable to load subbus library" );
  rv = read_ack( 0, &data );
  if ( rv ) nl_error(2, "Unexpected ACK reading from 0" );
  if ( argc < 2 )
    nl_error( 0, "Select from the following: idx, timeout, ana_in_cfg, ana_in_mux" );
  for (i = 1; i < argc; i++) {
    if ( strcmpi(argv[i], "idx") == 0 ) {
      indexer_test();
    } else if ( strcmp(argv[i], "timeout") == 0 ) {
      timeout_test();
    } else if ( strcmp(argv[i], "ana_in_cfg") == 0 ) {
      ana_in_cfg();
    } else if ( strcmp(argv[i], "ana_in_mux") == 0 ) {
      ana_in_mux();
    } else nl_error( 3, "Unrecognized test: '%s'", argv[i]);
  }

  return 0;
}
