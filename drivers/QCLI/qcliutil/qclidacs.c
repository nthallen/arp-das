#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "subbus.h"
#include "nortlib.h"
#include "qcliutil.h"
#include "oui.h"

static int board_number = 0;
static unsigned short board_base;
static unsigned short qcli_waddr, qcli_wsaddr, qcli_raddr;
static unsigned short qcli_wsaddr, qcli_vaddr;

static void sbwr_chk( unsigned short addr, unsigned short val ) {
  if (write_ack(addr, val) == 0)
    nl_error( 3, "No acknowledge writing to QCLI at 0x%03X",
      addr );
}

static unsigned short sbw_chk( unsigned short addr ) {
  unsigned short data;
  if (read_ack( addr, &data) == 0) {
    nl_error( 3, "No acknowledge reading from QCLI at 0x%03X",
        addr );
  }
  return data;
}

/* unsigned short read_qcli(int fresh);
   If fresh is non-zero, it means a command has just
   been issued, and we want to give the QCLI time
   to process that command before requesting status.
*/
unsigned short read_qcli( int fresh ) {
  delay(10);
  return sbw_chk(qcli_raddr);
}

void write_qcli( unsigned short value ) {
  sbwr_chk( qcli_waddr, value );
}

unsigned short wr_rd_qcli( unsigned short value ) {
  sbwr_chk( qcli_wsaddr, value );
  return read_qcli(1);
}

void wr_stop_qcli(  unsigned short value ) {
  sbwr_chk( qcli_wsaddr, value );
}

static unsigned short vbuf[32];

/**
 @return zero on success
 */
int verify_block( unsigned short addr, unsigned short *prog, int blocklen ) {
  static subbus_mread_req *vreq = 0;
  unsigned short ctrlr_status;
  unsigned short remaining;
  unsigned short blk_addr = addr;
  int rv = 0, retries = 0;
  time_t v_start, v_now;

  if ( vreq == 0 ) {
    char rbuf[15];
    snprintf(rbuf, 15, "%X@%X", 32, board_base+4);
    vreq = pack_mread_request( 32, rbuf);
  }
  ctrlr_status = sbw_chk(board_base);
  if ( ctrlr_status & 0x7FF ) {
    nl_error( 1,
      "Controller status before verify: 0x%04X, resetting",
      ctrlr_status);
    sbwr_chk(board_base+0xC,0);
    delay(10);
  }
  sbwr_chk( qcli_vaddr, addr ); // Request Verify
  v_start = time(NULL);
  for (;;) {
    ctrlr_status = sbw_chk(board_base);
    if ( ctrlr_status & 0x200 ) break;
    if (!(ctrlr_status & 0x100)) {
      nl_error( 2, "Controller not in read mode: %04X", ctrlr_status );
      return 1;
    }
    v_now = time(NULL);
    if ( difftime(v_now, v_start) > 3 ) {
      nl_error( 2, "Timeout waiting for verify on addr %04X. "
        "ctrlr_status: %04X", addr, ctrlr_status );
      if ( ++retries > 2 ) return 1;
      // Reset the controller and reissue the verify request
      sbwr_chk(board_base+0xC,0);
      delay(10);
      sbwr_chk( qcli_vaddr, addr ); // Request Verify
      v_start = time(NULL);
    }
  }
  remaining = ctrlr_status & 0xFF;
  if ( remaining != 128 )
    nl_error( 1, "Expected 128 bytes in read FIFO, reporting %u", remaining );
  while ( remaining > 0 ) {
    int i;
    if ( remaining < 32 ) {
      for ( i = 0; i < remaining; ++i ) {
        vbuf[i] = sbw_chk( board_base+4 );
      }
    } else {
      mread_subbus(vreq, vbuf);
    }
    for ( i = 0; i < remaining && i < 32; ++i ) {
      if ( i < blocklen && vbuf[i] != prog[addr+i] ) {
        nl_error( -2, "  %04X: program=%04X read=%04X",
          addr+i, prog[addr+i], vbuf[i] );
        ++rv;
      }
    }
    addr += i;
    // prog += i;
    remaining -= i;
    blocklen -= i;
    ctrlr_status = sbw_chk(board_base);
    if (remaining != (ctrlr_status & 0xFF))
      nl_error(2, "  Remaining count incorrect: %04X expected %02X",
        ctrlr_status, remaining);
  }
  if ( rv ) {
    nl_error(2, "Block 0x%04X: %d words failed on verify",
	blk_addr, rv );
  }
  return rv;

}

void qcli_addr_init( int bdnum ) {
  board_number = bdnum;
  board_base = QCLI_BASE + board_number * QCLI_INC;
  qcli_waddr = board_base + 6;
  qcli_wsaddr = board_base + 8;
  qcli_raddr = board_base + 2;
  qcli_vaddr = board_base + 0xA;
  sbw_chk( qcli_raddr );
  sbwr_chk( board_base+0xC, 0 ); // Reset the controller
}

void qclisrvr_init( int argc, char **argv ) {
  int c;
  int bdnum = 0;

  optind = OPTIND_RESET; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'd':
        bdnum = atoi(optarg);
        break;
      case '?':
        nl_error(3, "Unrecognized Option -%c", optopt);
    }
  }
  qcli_addr_init( bdnum );
}
