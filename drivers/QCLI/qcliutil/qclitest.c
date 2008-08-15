#include <stdlib.h>
#include "subbus.h"
#include "nortlib.h"
#include "qcliutil.h"
#include "oui.h"

#ifdef __USAGE
%C
  Runs diagnostics on QCLI V2
#endif

#ifndef FORMER_APPROACH
int main( int argc, char **argv ) {
  oui_init_options(argc, argv);
  /* if ( load_subbus() == 0 )
    	nl_error( 3, "No subbus library" );
  qcli_addr_init( 1 ); */
  if ( qcli_diags( 1 ) ) printf( "End of Tests\n" );
  else nl_error( 3, "Errors observed" );
  return 0;
}

#else
int main( int argc, char **argv ) {
  unsigned short qcli_status;

  if ( load_subbus() == 0 )
	nl_error( 3, "No subbus library" );
  qcli_addr_init( 1 );
  
  /* Verify that no errors are asserted. If any are,
     attempt to clear them */
  qcli_status = read_qcli(1);
  if ( qcli_status & QCLI_S_ERR ) {
	qcli_status = wr_rd_qcli( QCLI_CLEAR_ERROR );
  }
  if ( check_status( qcli_status, QCLI_S_ERR, 0,
		"Error status observed on startup", 1 ) )
	nl_error( 3, "Cannot continue" );

  /* Verify that QCLI is in Idle mode. If not, try
     to put it into Idle */
  printf( "Verifying Idle Status:\n" );
  if ( !( qcli_status & QCLI_S_IDLE ) ) {
	qcli_status = wr_rd_qcli( QCLI_STOP );
  }
  if ( check_status( qcli_status, QCLI_S_IDLE, QCLI_S_IDLE,
		"QCLI Not in Idle Mode after issuing STOP", 0 ) +
	   check_status( qcli_status, QCLI_S_MODE, 0,
		 "Mode bits are not Idle", 0 ) +
	   check_status( qcli_status, QCLI_S_ERR, 0,
	     "QCLI reports error after issuing STOP", 0 ) ) {
	report_status(qcli_status, "");
	nl_error( 3, "Cannot continue" );
  }

  /* Test the bad opcode error */
  printf( "Issuing invalid opcode:\n" );
  qcli_status = wr_rd_qcli( QCLI_BAD_CMD );
  if ( ! check_status( qcli_status, QCLI_S_CORDTE, QCLI_S_CORDTE,
		"CORDTE not observed", 1 ) ) {
	printf( "CORDTE Error Observed as expected\n" );
	qcli_status = wr_rd_qcli( QCLI_CLEAR_ERROR );
	if ( check_status( qcli_status, QCLI_S_CORDTE, 0,
			"CORDTE not cleared", 0 ) |
		 check_status( qcli_status, QCLI_S_ERR, 0,
		    "Error bits observed after CLEAR_ERROR", 0 )
		) report_status( qcli_status, "" );
	else printf( "CORDTE Cleared as expected\n" );
  }
  
  /* Test CMDERR by issuing Run - but only if we
     are not ready */
  if ( qcli_status & QCLI_S_READY ) {
	printf( "Skipping CMDERR test\n" );
  } else {
	printf( "Issuing PROGRAM_SECTOR from IDLE:\n" );
	qcli_status = wr_rd_qcli( QCLI_PROGRAM_SECTOR );
	if ( ! check_status( qcli_status, QCLI_S_CMDERR, QCLI_S_CMDERR,
		  "CMDERR not observed", 1 ) ) {
	  printf("CMDERR Observed\n");
	  qcli_status = wr_rd_qcli( QCLI_CLEAR_ERROR );
	  if ( !(check_status( qcli_status, QCLI_S_CMDERR, 0,
			"CMDERR not cleared", 1 ) ||
		   check_status( qcli_status, QCLI_S_ERR, 0,
			 "Error status observed after CLEAR_ERROR", 1 )))
		 printf( "CMDERR Cleared as expected\n" );
	}
  }
  
  printf( "Testing Program Mode:\n" );
  write_qcli( QCLI_LOAD_MSB | 0 );
  qcli_status = wr_rd_qcli( QCLI_WRITE_ADDRESS | 0 );
  if ( check_status( qcli_status, QCLI_S_IDLE, 0,
		"IDLE bit not cleared after WRITE_ADDRESS", 0 ) |
	   check_status( qcli_status, QCLI_S_PROGRAM, QCLI_S_PROGRAM,
	    "PROGRAM bit not set after WRITE_ADDRESS", 0 ) |
	   check_status( qcli_status, QCLI_S_MODE, 1,
	    "MODE bits != 1 after WRITE_ADDRESS", 0 ) |
	   check_status( qcli_status, QCLI_S_READY, 0,
	    "READY still set after WRITE_ADDRESS", 0 ) |
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
	"CHECKSUM bit no cleared after WRITE_ADDRESS", 1 );
  
  qcli_status = wr_rd_qcli( QCLI_STOP );
  check_status( qcli_status, QCLI_S_IDLE, QCLI_S_IDLE,
	"Not IDLE after STOP", 1 );

  printf( "End of tests\n" );
  return 0;
}
#endif
