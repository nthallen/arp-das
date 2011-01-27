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

#define aicfg_not 0
#define aicfg_hiz 1
#define aicfg_null 2
#define aicfg_2 3
#define aicfg_1 4
#define aicfg_p768 5
#define aicfg_p384 6
#define aicfg_p192 7
#define aicfg_p096 8
#define aicfg_muxed 9
#define aicfg_other 10

static char *gain_text[] = {
  "Unconfigured",
  "Hi-Z ",
  "Null ",
  "X2   ",
  "X1   ",
  "X.768",
  "X.384",
  "X.192",
  "X.096",
  "Muxed",
  "Other" };

static int gain_type( int cfg ) {
  if ( cfg & 0x100 ) return aicfg_muxed;
  else if ( cfg & 1 ) return aicfg_hiz;
  else if (cfg & 2) return aicfg_null;
  else switch ((cfg & 0x1C) >> 2) {
    case 0: return aicfg_p096;
    case 2: return aicfg_p192;
    case 4: return aicfg_p384;
    case 6: return aicfg_p768;
    case 5: return aicfg_1;
    case 7: return aicfg_2;
    default: return aicfg_other;
  }
}

static void ana_in_read_cfg(void) {
  int i;
  int gain[256];
  unsigned short cfgs[256];
  unsigned short srcaddr[16];

  for (i = 0; i < 256; ++i) gain[i] = aicfg_not;
  for (i = 0; i < 16; ++i) srcaddr[i] = 0;
  for (i = 0; i < 128; ++i) {
    unsigned short addr, data, cfg;
    addr = 0xC00 + 2*i;
    data = sbrwa( addr );
    cfg = sbrwa( addr + 1);
    cfgs[i] = cfg;
    gain[i] = gain_type(cfg);
    if ( gain[i] == aicfg_muxed) {
      int base, j, muxno;
      muxno = ((cfg&0xE0)>>4) + ((i&8)>>3);
      base = 0xD00 + (muxno<<4);
      if (srcaddr[muxno]) {
	nl_error( 1, "Channel %03X muxed to %03X: already in use by %03X",
	  addr, base, srcaddr[muxno] );
      }
      srcaddr[muxno] = addr;
      for (j = 0; j < 7; j++) {
	unsigned short maddr = base + 2*j;
	int n = 128 + (muxno<<3) + j;
	data = sbrwa( maddr );
	cfg = sbrwa( maddr + 1);
	if (gain[n] != aicfg_not)
	  nl_error(3, "Channel %03X previously read");
	if ( !(cfg & 0x100) )
	  nl_error(2, "Muxed channel %03X (from %03X) not reporting mux cfg",
	    addr, maddr );
	gain[n] = gain_type( cfg & 0x1F );
      }
    }
  }
  for (i = 1; i < aicfg_muxed; ++i) {
    int j, k;
    int one = 0;
    printf( "%s:", gain_text[i] );
    for (j = 0; j < 256; ) {
      if ( gain[j] == i ) {
	printf("%s %03X", one ? "," : "", 0xC00 + 2*j);
	if ( gain[j] == i ) {
	  for (k = j; k < 255 && gain[k+1] == i; ++k);
	  if (k > j) printf( "-%03X", 0xC00 + 2*k );
	}
	j = k+1;
	one = 1;
      } else ++j;
    }
    printf( "%s\n", one ? "" : " none" );
  }
  printf( "%s:", gain_text[aicfg_muxed]);
  { int one = 0, j;
    for (j = 0; j < 128; ++j) {
      if (gain[j] == aicfg_muxed) {
	unsigned short cfg = cfgs[j];
	int muxno = ((cfg&0xE0)>>4) + ((i&8)>>3);
	unsigned short maddr = 0xD00 + (muxno<<4);
	printf("%s %03X => %03X-E", one ? "," : "",
	  0xC00+2*j, maddr );
	one = 1;
      }
    }
    printf( "%s\n", one ? "" : " none" );
  }
  printf( "%s:", gain_text[aicfg_other]);
  { int one = 0, j;
    for (j = 0; j < 128; ++j) {
      if (gain[j] == aicfg_other) {
	unsigned short cfg = cfgs[j];
	printf("%s %03X(0x%03X)", one ? "," : "",
	  0xC00+2*j, cfg );
	one = 1;
      }
    }
    printf( "%s\n", one ? "" : " none" );
  }

}

int main(int argc, char **argv) {
  int rv, i;
  unsigned short data;

  if ( load_subbus() == 0 )
    nl_error( 3, "Unable to load subbus library" );
  rv = read_ack( 0, &data );
  if ( rv ) nl_error(2, "Unexpected ACK reading from 0" );
  if ( argc < 2 )
    nl_error( 0, "Select from the following: \n"
	"  idx\n"
	"  timeout\n"
	"  ana_in_cfg\n"
	"  ana_in_mux\n"
	"  ana_in_read_cfg" );
  for (i = 1; i < argc; i++) {
    if ( strcmpi(argv[i], "idx") == 0 ) {
      indexer_test();
    } else if ( strcmp(argv[i], "timeout") == 0 ) {
      timeout_test();
    } else if ( strcmp(argv[i], "ana_in_cfg") == 0 ) {
      ana_in_cfg();
    } else if ( strcmp(argv[i], "ana_in_mux") == 0 ) {
      ana_in_mux();
    } else if ( strcmp(argv[i], "ana_in_read_cfg") == 0 ) {
      ana_in_read_cfg();
    } else nl_error( 3, "Unrecognized test: '%s'", argv[i]);
  }

  return 0;
}
