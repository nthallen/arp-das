/* Copyright 2001 by the President and Fellows of Harvard College */
#include "tm.h"

static void tm_data2( int type, int n_rows, mfc_t mfctr, int row,
                     const unsigned char *data ) {
  while ( n_rows-- > 0 ) {
    if ( row == tm_info.nrowminf ) row = 0;
    if ( row == 0 ) {
	  mfc_t mfcchk;
	  mfcchk = data[tmi(mfc_lsb)] + ( data[tmi(mfc_msb)] << 8 );
	  if ( type == TMTYPE_DATA_T1 ) mfctr = mfcchk;
	  else if ( mfcchk != ++mfctr ) {
	    nl_error( 2, "Invalid mfctr in tm_data2" );
	    return;
	  }
	}
	if ( row+1 == tm_info.nrowminf ) {
	  unsigned short synch =
	    *((unsigned short *)(data+tmi(nbrow)-2));
	  if ( (tmi(flags) & TMF_INVERTED ) && row+1 == tmi(nrowmajf) ) {
	    if ( ~synch != tmi(synch) ) {
		  nl_error( 2, "Invalid inverted synch in tm_data2" );
		  return;
	    }
	  } else if ( synch != tmi(synch) ) {
	    nl_error( 2, "Invalid synch in tm_data2" );
	    return;
	  }
	}
    TM_row( mfctr, row++, data );
    data += tmi(nbrow);
  }
}

void tm_data3( int n_rows, mfc_t mfctr, const unsigned char *data ) {
  int offset = tmi(nbrow) - 4;
  data = data-2;
  while ( n_rows-- > 0 ) {
    TM_row( mfctr++, 0, data );
    data += offset;
  }
}

void TM_data( tm_msg_t *msg, int n_bytes ) {
  switch ( msg->hdr.tm_type ) {
    case TMTYPE_DATA_T1:
      tm_data2( TMTYPE_DATA_T1, msg->body.data1.n_rows, 0, 0,
                msg->body.data1.data );
      break;
    case TMTYPE_DATA_T2:
      tm_data2( TMTYPE_DATA_T2, msg->body.data2.n_rows,
				msg->body.data2.mfctr, msg->body.data2.rownum,
				msg->body.data1.data );
      break;
    case TMTYPE_DATA_T3:
      tm_data3( msg->body.data3.n_rows, msg->body.data3.mfctr,
                msg->body.data3.data );
	  break;
    case TMTYPE_DATA_T4:
      /* Test the crc */
      tm_data3( msg->body.data4.n_rows, msg->body.data4.mfctr,
                msg->body.data4.data );
	  break;
	default: nl_error( 4, "Bad type in TM_data" );
  }
}
