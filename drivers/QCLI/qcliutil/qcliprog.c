/* qcliprog.c */
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "nortlib.h"
/* #include "subbus.h" */
#include "oui.h"
#include "qcliutil.h"
#include "qcliprog.h"

int opt_vw = 0, n_opts = 0;
#define OPT_V 0
#define OPT_W 1

char *ifilename;
FILE *ifile;

void qcliprog_init( int argc, char **argv ) {
  int c;

  optind = OPTIND_RESET; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'w': opt_vw |= (OPT_W << n_opts++); break;
      case 'c': opt_vw |= (OPT_V << n_opts++); break;
      case '?':
        nl_error(3, "Unrecognized Option -%c", optopt);
    }
  }
  if ( n_opts == 0 ) { // default to verify
    n_opts = 1;
    opt_vw = OPT_V;
  }
  if ( optind < argc )
    ifilename = argv[optind++];
  if ( optind < argc )
    nl_error( 3, "Too many input arguments" );
  if ( ifilename != 0 ) {
    ifile = fopen( ifilename, "r" );
    if ( ifile == 0 )
      nl_error( 3, "Unable to open input file '%s'", ifilename );
  } else {
    ifilename = "<stdin>";
    ifile = stdin;
  }
}

#define IBUFSIZE 40
static char ibuf[IBUFSIZE+1];
static char *ibufp;
static int ibufstart, ibufend, ibufeol = -1;
/* ibufstart is offset to start of line
   ibufeol is offset of end-of-line which
     is set to NUL
   ibufend is offset beyond last char
*/

/* qcli_readline reads a line of text from ifile
   into ibuf, replacing the newline with a NUL.
   If the line length exceeds IBUFSIZE, the
   excess is discarded after returning the
   truncated line. If required is set and EOF
   is reached, qcli_readline issues a fatal
   error. If no EOL is found, ibufeol is set
   to -1.
*/
int qcli_readline( int required ) {
  while ( ibufeol < 0 && ibufend > 0 ) {
    ibufend = ibufstart = 0;
    ibufeol = -1;
    if ( qcli_readline( required ) == 0 ) return 0;
  }
  ibufstart = ++ibufeol;
  for (;;) {
    int nb;
    if ( ibufend >= ibufeol ) {
      for ( ; ibufeol < ibufend && ibuf[ibufeol] != '\n';
            ibufeol++);
      ibuf[ibufeol] = '\0';
      if ( ibufeol < ibufend ) {
        ibufp = ibuf+ibufstart;
        return 1;
      } else if ( ibufstart == 0 && ibufend == IBUFSIZE ) {
        ibufeol = -1;
        ibufp = ibuf;
        return 1;
      } else {
        memmove( ibuf, ibuf+ibufstart, ibufend-ibufstart );
        ibufend -= ibufstart;
        ibufeol -= ibufstart;
        ibufstart = 0;
      }
    }
    nb = fread( ibuf+ibufend, 1, IBUFSIZE-ibufend, ifile );
    ibufend += nb;
    if ( ibufstart == ibufend ) {
      if ( required )
        nl_error( 3, "Expected an input line in qcli_readline" );
      return 0;
    }
  }
}

void qcli_readnum( unsigned short *value, unsigned short *count ) {
  unsigned long ival;
  char *ep;
  
  do qcli_readline(1); while ( *ibufp == ':' );
  if ( ! isxdigit(*ibufp) )
    nl_error( 3, "Unexpected non-hexdigit in qcli_readnum: '%s'",
              ibufp );
  ival = strtoul( ibufp, &ep, 16 );
  if ( ival > USHRT_MAX )
    nl_error( 3, "Value out of range in qcli_readnum: '%s'",
              ibufp );
  *value = (unsigned short) ival;
  if ( *ep == ':' ) {
    *count = 1;
  } else if ( strncmp( ep, " x ", 3 ) != 0 ||
              ! isdigit( ep[3] ) ) {
    nl_error( 3, "Syntax error in qcli_readnum: '%s'",
              ibufp );
  } else {
    ival = strtoul( ep+3, NULL, 10 );
    if ( ival > USHRT_MAX )
      nl_error( 3, "Count way out of range in qcli_readnum: '%s'",
                ibufp );
    *count = (unsigned short) ival;
  }
}

unsigned short *load_program( long *proglenp ) {
  long proglen;
  unsigned short value, count;
  unsigned short *prog, ip = 0;
  
  while ( qcli_readline(1) ) {
    if ( strcmp( ibufp, "Compiled Output:" ) == 0 )
      break;
  }
  qcli_readnum( &value, &count );
  if ( count != 1 )	nl_error( 3, "First count != 1!" );
  proglen = value ? value : 65536L;
  prog = new_memory( proglen * sizeof(unsigned short) );
  prog[ip++] = proglen;
  while ( ip < proglen ) {
    qcli_readnum( &value, &count );
    if ( ip+count > proglen )
      nl_error( 3, "Data exceeds program length" );
    while ( count-- != 0 ) prog[ip++] = value;
  }
  fclose(ifile);
  *proglenp = proglen;
  return prog;
}

int write_block( unsigned short addr, unsigned short *prog, int blocklen ) {
  unsigned short chksum = 0, qcli_status;
  unsigned short startaddr = addr;
  unsigned short last_word = prog[addr];
  qcli_status = read_qcli(0);
  if (qcli_status & QCLI_S_FWERR) {
    unsigned short new_status;
    new_status = wr_rd_qcli(QCLI_CLEAR_ERROR);
    if (new_status & QCLI_S_FWERR) {
      report_status( qcli_status );
      nl_error(2, "0x%04X: Firmware error before write, did not clear",
        startaddr);
      return 1;
    } else {
      report_status( qcli_status );
      nl_error(1, "0x%04X: Firmware error reported before write, cleared",
        startaddr );
    }
  }
  write_qcli( QCLI_LOAD_MSB | ((addr>>8)&0xFF) );
  write_qcli( QCLI_WRITE_ADDRESS | (addr&0xFF) );
  while ( blocklen-- > 0 ) {
    unsigned short value = prog[addr++];
    write_qcli( QCLI_LOAD_MSB | ((value>>8)&0xFF) );
#ifdef EXTRA_CHECKS
    qcli_status = wr_rd_qcli( QCLI_WRITE_DATA | (value&0xFF) );
    if (qcli_status & QCLI_S_FWERR) {
      report_status(qcli_status);
      nl_error(2, "0x%04X: Firmware error during write", addr);
      return 1;
    }
#else
    write_qcli( QCLI_WRITE_DATA | (value&0xFF) );
#endif
    chksum += value;
  }
  chksum = -chksum;
  write_qcli( QCLI_LOAD_MSB | ((chksum>>8)&0xFF) );
  qcli_status = wr_rd_qcli( QCLI_WRITE_CHKSUM | (chksum&0xFF) );
  if ( qcli_status & QCLI_S_CHKSUM ) {
    nl_error( 2, "0x%04X - CHKSUM failed after write", startaddr );
    return 1;
  }
  if ( qcli_status & QCLI_S_FWERR ) {
    report_status( qcli_status );
    nl_error( 2, "0x%04X: Firmware error reported after write", startaddr );
    return 1;
  }
  qcli_status = wr_rd_qcli( QCLI_PROGRAM_SECTOR );
  if ( (qcli_status & QCLI_S_MODE) == QCLI_PSECTOR_MODE ) {
    // printf( "I actually saw Program Sector Mode!\n" );
    qcli_status = read_qcli(1);
  }
  if ( (qcli_status & QCLI_S_MODE) != QCLI_PROGRAM_MODE ) {
    report_status( qcli_status );
    nl_error( 2, "0x%04X: Expected PROGRAM Mode after program sector command",
                  startaddr );
    return 1;
  }
  /* Go into read mode to monitor TGL and DATA lines */
  write_qcli(QCLI_LOAD_MSB | ((startaddr>>8) & 0xFF));
  write_qcli(QCLI_WRITE_ADDRESS | (startaddr & 0xFF));
  qcli_status = wr_rd_qcli(QCLI_READ_DATA);
  { int nreads = 20; /* Up from 10 just to see... */
    int nmatch = 0;
    while (nreads-- > 0) {
      if ((qcli_status & (QCLI_S_FLSHDATA|QCLI_S_FLSHTGL)) ==
          (last_word & (QCLI_S_FLSHDATA|QCLI_S_FLSHTGL))) {
        if (++nmatch >= 5) break;
      } else {
        nmatch = 0;
        qcli_status = read_qcli(0);
      }
    }
    /* STOP in order to get out of read mode and get
       actual status */
    qcli_status = wr_rd_qcli(QCLI_STOP);
    if ( nreads < 0 ) {
      report_status( qcli_status );
      nl_error( 2, "0x%04X: Valid data never observed", startaddr );
      return 1;
    } else if ( qcli_status & QCLI_S_FWERR ) {
      report_status( qcli_status );
      nl_error( 2, "0x%04X: Firmware error detected after program sector",
                    startaddr);
      return 1;
    }
  }
  nl_error(0, "Wrote block at addr 0x%04X", startaddr );
  fflush(stdout);
  return 0;
}

void write_program( unsigned short *prog, long proglen ) {
  unsigned short addr = 0;
  while ( proglen > 0 ) {
    int blocklen = proglen > 128 ? 128 : proglen;
    write_block( addr, prog, blocklen );
    proglen -= blocklen;
    addr += blocklen;
  }
  wr_stop_qcli( QCLI_STOP );
}

/* returns zero if everything checks out */
int verify_program( unsigned short *prog, long proglen ) {
  unsigned short addr = 0;
  int rv = 0;
  while ( proglen > 0 ) {
    int blocklen = proglen > 128 ? 128 : proglen;
    if ( verify_block( addr, prog, blocklen ) )
      rv = 1;
    proglen -= blocklen;
    prog += blocklen;
    addr += blocklen;
  }
  return rv;
}

void write_verify_program(unsigned short *prog, long proglen) {
  unsigned short addr = 0;

  while ( proglen > 0 ) {
    int ln_opts = n_opts;
    int lopt_vw = opt_vw;
    int blocklen = proglen > 128 ? 128 : proglen;
    int block_unverified = 1;
    int block_verify_failed = 0;
    while (ln_opts && block_unverified) {
      --ln_opts;
      switch (lopt_vw & 1) {
        case OPT_W:
          if (write_block(addr, prog, blocklen)) {
            int i;
            for (i = 0; i < 4; ++i) {
              if (qcli_diags(0)) break;
            }
          }
          break;
        case OPT_V:
          if (verify_block( addr, prog, blocklen)) {
            block_verify_failed = 1;
          } else {
            block_unverified = 0;
            if (block_verify_failed)
              nl_error(0, "Block 0x%04X: verified", addr);
          }
          break;
      }
      lopt_vw >>= 1;
    }
    if (block_unverified)
      nl_error(3, "Programming and/or verification of block 0x%04X failed",
                  addr);
    proglen -= blocklen;
    addr += blocklen;
  }
  nl_error(0, "Program written and/or verified completely");
}

int main( int argc, char **argv ) {
  long proglen;
  unsigned short *prog;
  int i;
  
  oui_init_options( argc, argv );
  prog = load_program( &proglen );
  nl_error(0, "Loaded %ld words from %s",
    proglen, ifilename );
  for (i = 0; i < 4; ++i) {
    if ( qcli_diags( 0 ) ) break;
  }
  if (i < 4) {
    if (i > 0) nl_error( 1, "Diagnostics passed after %d failure(s)", i );
    else nl_error(0, "Diagnostics passed on first attempt");
  } else nl_error(3, "Errors observed during diagnostics over %d attempts", i);
  write_verify_program(prog, proglen);
  return 0;
}
