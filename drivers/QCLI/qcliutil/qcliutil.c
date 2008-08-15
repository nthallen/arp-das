#include <time.h>
// #include <i86.h>
#include "nortlib.h"
#include "qcliutil.h"

/* #define VERBOSE_READ 1 */
#define N_VERBOSE_READS 5
#define USE_DELAY 1

typedef struct {
  unsigned short bits;
  char *text;
} bitdef;
bitdef qcli_bits[] = {
  QCLI_S_BUSY, "BUSY",
  QCLI_S_CHKSUM, "CHKSUM",
  QCLI_S_CMDERR, "CMDERR",
  QCLI_S_LASEROFF, "LASERON",
  QCLI_S_CORDTE, "CORDTE",
  QCLI_S_READY, "READY",
  QCLI_S_WAVEERR, "WAVEERR",
  /* QCLI_S_FLSHDATA, "FLSHDATA", */
  /* QCLI_S_FLSHTGL, "FLSHTGL", */
  QCLI_S_DOT, "DOT",
  QCLI_S_LOT, "LOT",
  QCLI_S_LOC, "LOC",
  0, 0
};

void report_status( unsigned short status, char *str ) {
  int i;
  printf( "  Status = 0x%04X\n  ", status );
  if ( str != 0 && *str != 0 )
    printf( "%s\n  ", str );
  status ^= QCLI_S_LASEROFF;
  for ( i = 0; qcli_bits[i].bits; i++ ) {
	if ( status & qcli_bits[i].bits )
	  printf( " %s", qcli_bits[i].text );
  }
  printf( "\n  Mode bits: %d\n", status & QCLI_S_MODE );
}

static int qcli_error_reported = 0;

/* Returns zero if status matches. Otherwise prints the
   text and optionally dumps the status word. */
int check_status( unsigned short status, unsigned short mask,
		unsigned short value, char *text, int dump ) {
  if ( (status & mask) == value ) return 0;
  qcli_error_reported = 1;
  nl_error( 2, "%s", text );
  if ( dump ) report_status( status, "" );
  return 1;
}

void delay3msec(void) {
  #ifdef USE_DELAY
	delay(3);
  #else
	struct timespec tp;
	tp.tv_sec = 0;
	tp.tv_nsec = 3000000L;
	if ( nanosleep( &tp, NULL ) != 0 )
	  nl_error( 3, "nano_sleep saw signal" );
  #endif
}

/* qcli_diags(verbose)
   Returns non-zero on success, zero in case of errors
   verbose:
     0 - prints errors only
	 1 - prints progress and errors
*/
int qcli_diags( int verbose ) {  
  unsigned short qcli_status;

  qcli_error_reported = 0;

  /* Verify that no errors are asserted. If any are,
     attempt to clear them */
  if (verbose) printf("QCLI Diagnostics\n");
  qcli_status = wr_rd_qcli( QCLI_STOP );
  if ( qcli_status & QCLI_S_ERR ) {
	qcli_status = wr_rd_qcli( QCLI_CLEAR_ERROR );
  }
  if ( check_status( qcli_status, QCLI_S_FWERR, 0,
		"Error status observed on startup", 1 ) )
	return 0;

  /* Verify that QCLI is in Idle mode. If not, try
     to put it into Idle */
  if ( verbose ) printf( "Verifying Idle Status:\n" );
  if ( ( qcli_status & QCLI_S_MODE ) != QCLI_IDLE_MODE ) {
	qcli_status = wr_rd_qcli( QCLI_STOP );
  }
  if ( check_status( qcli_status, QCLI_S_MODE, QCLI_IDLE_MODE,
		"QCLI Not in Idle Mode after issuing STOP", 0 ) +
	   check_status( qcli_status, QCLI_S_FWERR, 0,
	     "QCLI reports error after issuing STOP", 0 ) ) {
	if ( verbose) report_status(qcli_status, "");
	return 0;
  }

  /* Test the bad opcode error */
  if ( verbose ) printf( "Issuing invalid opcode:\n" );
  qcli_status = wr_rd_qcli( QCLI_BAD_CMD );
  if ( ! check_status( qcli_status, QCLI_S_CORDTE, QCLI_S_CORDTE,
		"CORDTE not observed", 1 ) ) {
	if (verbose) printf( "CORDTE Error Observed as expected\n" );
	qcli_status = wr_rd_qcli( QCLI_CLEAR_ERROR );
	if ( check_status( qcli_status, QCLI_S_CORDTE, 0,
			"CORDTE not cleared", 0 ) |
		 check_status( qcli_status, QCLI_S_FWERR, 0,
		    "Error bits observed after CLEAR_ERROR", 0 )
		) report_status( qcli_status, "" );
	else if (verbose) printf( "CORDTE Cleared as expected\n" );
  }
  
  /* Test CMDERR by issuing Run - but only if we
     are not ready */
  if ( qcli_status & QCLI_S_READY ) {
	if (verbose) printf( "Skipping CMDERR test\n" );
  } else {
	if (verbose) printf( "Issuing PROGRAM_SECTOR from IDLE:\n" );
	qcli_status = wr_rd_qcli( QCLI_PROGRAM_SECTOR );
	if ( ! check_status( qcli_status, QCLI_S_CMDERR, QCLI_S_CMDERR,
		  "CMDERR not observed", 1 ) ) {
	  if (verbose) printf("CMDERR Observed\n");
	  qcli_status = wr_rd_qcli( QCLI_CLEAR_ERROR );
	  if ( !(check_status( qcli_status, QCLI_S_CMDERR, 0,
			"CMDERR not cleared", 1 ) ||
		   check_status( qcli_status, QCLI_S_FWERR, 0,
			 "Error status observed after CLEAR_ERROR", 1 ))) {
		 if (verbose) printf( "CMDERR Cleared as expected\n" );
	  }
	}
  }
  
  if (verbose) printf( "Testing Program Mode:\n" );
  write_qcli( QCLI_LOAD_MSB | 0 );
  qcli_status = wr_rd_qcli( QCLI_WRITE_ADDRESS | 0 );
  if ( check_status( qcli_status, QCLI_S_MODE, QCLI_PROGRAM_MODE,
	    "Not in PROGRAM Mode after WRITE_ADDRESS", 0 ) |
	   check_status( qcli_status, QCLI_S_CHKSUM, 0,
		 "CHECKSUM bit set after WRITE_ADDRESS", 0 ) )
	report_status( qcli_status, "" );
  qcli_status = wr_rd_qcli( QCLI_WRITE_DATA | 0x55 );
  check_status( qcli_status, QCLI_S_CHKSUM, QCLI_S_CHKSUM,
	"CHKSUM bit not set after writing data", 1 );
  write_qcli( QCLI_LOAD_MSB | 0xFF );
  qcli_status = wr_rd_qcli( QCLI_WRITE_DATA | 0xAB );
  check_status( qcli_status, QCLI_S_CHKSUM, 0,
	"CHKSUM bit not cleared after writing complementary data", 1 );
  write_qcli( QCLI_LOAD_MSB | 0x55 );
  qcli_status = wr_rd_qcli( QCLI_WRITE_DATA | 0x55 );
  check_status( qcli_status, QCLI_S_CHKSUM, QCLI_S_CHKSUM,
	"CHKSUM bit not set after writing 3rd data word", 1 );
  write_qcli( QCLI_LOAD_MSB | 0xAA );
  qcli_status = wr_rd_qcli( QCLI_WRITE_CHKSUM | 0xAB );
  check_status( qcli_status, QCLI_S_CHKSUM, 0,
	"CHKSUM bit not cleared after writing complementary checksum", 1 );
  qcli_status = wr_rd_qcli( QCLI_WRITE_DATA | 0x55 );
  check_status( qcli_status, QCLI_S_CHKSUM, QCLI_S_CHKSUM,
	"CHKSUM bit not set after writing 4th data word", 1 );
  write_qcli( QCLI_LOAD_MSB | 0 );
  qcli_status = wr_rd_qcli( QCLI_WRITE_ADDRESS | 0 );
  check_status( qcli_status, QCLI_S_CHKSUM, 0,
	"CHECKSUM bit not cleared after WRITE_ADDRESS", 1 );
  
  qcli_status = wr_rd_qcli( QCLI_STOP );
  check_status( qcli_status, QCLI_S_MODE, QCLI_IDLE_MODE,
	"Not IDLE after STOP", 1 );

  return !qcli_error_reported;
}
