#include "nortlib.h"
#include "qcliutil.h"
#include "qcliprog.h"

/* Returns zero if block verifies as OK */
int verify_block( unsigned short addr, unsigned short *prog, int blocklen ) {
  int rv = 0;
  unsigned short block_addr = addr;
  if ( (addr&0xFF) + blocklen > 256 )
    nl_error( 4, "Invalid addr/blocklen pair: %04X, %d",
          addr,	blocklen );
  write_qcli( QCLI_LOAD_MSB | ((addr>>8)&0xFF) );
  write_qcli( QCLI_WRITE_ADDRESS | (addr&0xFF) );
  write_qcli( QCLI_READ_DATA );
  for ( ; blocklen-- > 0; addr++ ) {
    unsigned short value;
    unsigned short attempts;
    for ( attempts = 0; attempts < 5; ) {
      attempts++;
      value = wr_rd_qcli( QCLI_WRITE_ADDRESS | (addr&0xFF) );
      if ( value == prog[addr] ) break;
    }
    if ( value != prog[addr] ) {
      nl_error( -2, "  %04X: program=%04X read=%04X after %d attempts",
        addr, prog[addr], value, attempts );
      ++rv;
    } else if ( attempts > 1 ) {
      nl_error( 1, "  %04X: program=%04X required %d attempts",
        addr, prog[addr], attempts );
    }
  }
  wr_stop_qcli( QCLI_STOP );
  if (rv) nl_error(2, "Block %04X: %d words failed on verify",
              block_addr, rv);
  return rv;
}
