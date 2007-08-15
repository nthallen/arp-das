/* Copyright 2001 by the President and Fellows of Harvard University */
#include "nortlib.h"
#include "tm.h"

int TM_fd = -1;
char *TM_buf;

int TM_read_stream( int optimal ) {
  if ( TM_fd < 0 && TM_open_stream( optimal ) ) return 1;
  return TM_readfd();
}

int TM_readfd(void) {
  if ( TM_fd < 0 ) {
    nl_error( nl_response, "TM stream not opened in TM_readfd" );
    return 1;
  }
  if ( TM_buf == 0 ) {
    TM_buf = (char *) malloc( TMBUFSIZE );
    if ( TM_buf == 0 ) {
      nl_error( nl_response, "No memory for TM_buf in TM_open_stream" );
      return 1;
    }
  }
  for (;;) {
    ssize_t nbytes = read( TM_fd, TM_buf, TMBUFSIZE );
    if ( nbytes == 0 ) {
      if ( eof(TM_fd ) ) TM_stream( 0, TM_buf );
      return 0;
    }
    TM_stream( nbytes, TM_buf );
  }
}

static char hdr_array[] = "TM";
/* TM_stream strategy:
  buf_offset records a byte offset into ubuf where new
  data should be written except for two special cases:
  buf_offset == -1 indicates some sort of input data error
    for which TM_stream will try to recover by scanning ahead
    for the 'TM' header characters.
  buf_offset == -2 indicates we have received EOF (nbytes==0).

  want indicates how many bytes are currently required in the
  header of ubuf before we can process the current packet.
  want == 0 indicates that we are processing the tm_id
  characters. In this case, buf_offset should be -1, 0 or 1.
  Once we match the tm_id, want is advanced to 4 to get
  the tmtype code, then it is advanced again depending on
  the type. For data types, one final advance is required
  to collect the data itself.
*/
int TM_stream( int nbytes, const char *data ) {
  static tm_type;
  static tm_packet_t *ubuf = NULL;
  static int buf_offset = 0;
  static int want = 0, to_complete;
  int msg_offset = 0;
  
  if ( ubuf == 0 ) {
    ubuf = (tm_packet_t *)malloc( sizeof(tm_packet_t) );
    if ( ubuf == 0 ) nl_error( 3, "No memory for TM_stream buffer" );
    ubuf->msg.hdr.tm_id = TMHDR_WORD;
  }
  if ( buf_offset < -1 ) nl_error( 3, "TM_stream called after EOF" );
  if ( nbytes == 0 ) {
    if ( want != 0 ) nl_error( 2, "EOF found mid-packet" );
    TM_quit();
    buf_offset = -2;
    return 0;
  }
  for (;;) {
    if ( want == 0 ) {
      for (;;) {
        if ( msg_offset >= nbytes ) return 0;
        if ( buf_offset >= 0 ) {
          if ( data[msg_offset++] == hdr_array[buf_offset++] ) {
            if ( buf_offset == 2 ) {
              want = 4;
              break;
            }
          } else {
            nl_error( 2, "Frame error in TM_stream, skipping..." );
            buf_offset = -1;
          }
        } else if ( data[msg_offset++] == hdr_array[0] )
          buf_offset = 1;
      }
    }
    while ( want && msg_offset < nbytes ) {
      int bytes_wanted = want - buf_offset;
      int bytes_avail = nbytes - msg_offset;
      int bytes_copy = min( bytes_wanted, bytes_avail );
      memcpy( &ubuf->raw[buf_offset], &data[msg_offset], bytes_copy );
      msg_offset += bytes_copy;
      buf_offset += bytes_copy;
      if ( buf_offset == want ) {
        if ( want == 4 ) {
          to_complete = 0;
          switch ( ubuf->msg.hdr.tm_type ) {
            case TMTYPE_INIT: want += sizeof( tm_info_t ); break;
            case TMTYPE_TSTAMP: want += sizeof( tstamp_t ); break;
            case TMTYPE_DATA_T1: want += sizeof( tm_data_t1_t ) - 2; break;
            case TMTYPE_DATA_T2: want += sizeof( tm_data_t2_t ) - 2; break;
            case TMTYPE_DATA_T3: want += sizeof( tm_data_t3_t ) - 2; break;
            case TMTYPE_DATA_T4: want += sizeof( tm_data_t4_t ) - 2; break;
            default:
              nl_error( 2, "Invalid tmtype in TM_stream, skipping..." );
              buf_offset = -1;
              want = 0;
              break;
          }
        } else if ( to_complete ) {
          switch ( ubuf->msg.hdr.tm_type ) {
            case TMTYPE_DATA_T1:
            case TMTYPE_DATA_T2:
            case TMTYPE_DATA_T3:
            case TMTYPE_DATA_T4:
              TM_data( &ubuf->msg, want );
              break;
            default: nl_error( 4, "Invalid tmtype 3" );
          }
          want = 0; buf_offset = 0;
        } else {
          switch ( ubuf->msg.hdr.tm_type ) {
            case TMTYPE_INIT: 
              memcpy( &tm_info, &ubuf->msg.body.init, sizeof(tm_info_t) );
              TM_init();
              want = 0; buf_offset = 0; break;
            case TMTYPE_TSTAMP: 
              tm_info.t_stmp = ubuf->msg.body.ts;
              TM_tstamp( TMTYPE_TSTAMP, ubuf->msg.body.ts.mfc_num,
                ubuf->msg.body.ts.secs );
              want = 0; buf_offset = 0; break;
            case TMTYPE_DATA_T1: 
            case TMTYPE_DATA_T2:
              want += ubuf->msg.body.data1.n_rows * tm_info.tm.nbrow;
              break;
            case TMTYPE_DATA_T3:
            case TMTYPE_DATA_T4:
              want += ubuf->msg.body.data1.n_rows *
                ( tm_info.tm.nbrow - 4 );
              break;
            default:
              nl_error( 4, "Invalid tmtype after want" );
          }
          to_complete = 1;
          if ( want > TMBUFSIZE ) {
            nl_error( 2,
               "Invalid data record: specified size too large, skipping..." );
            want = 0; buf_offset = -1; break;
          }
        }
      }
    }
  }
}
/*
=Name TM_stream(): Telemetry Stream Parser
=Subject TM Functions
=Synopsis
#include "tm.h"
int TM_stream( int nbytes, const char *data );

=Description

  TM_stream() accepts blocks of data which have been
  read from a TM stream or generated by some other means
  and parses them into TM message blocks, performing
  whatever checking is relevant. TM_stream() in turn
  calls =TM_init=(), =TM_data=(), =TM_tstamp=() and
  =TM_quit=() at appropriate times within the stream.
  While basic forms of these functions are supplied by
  the library, most useful programs will want to supply
  customized versions of one or more of these.

  =TM_init=() is called once when the initial TM definition
  structure has been received. It will be called before
  any of the other routines are called. The default function
  does nothing.

  =TM_data=() is called whenever telemetry frame data is
  received. The library version of TM_data() will in turn
  call =TM_row=() for each row of data. Programs that are
  not concerned with data at the row level, such as
  a logger program, will choose to replace this function.

  =TM_tstamp=() is called whenever a new timestamp is
  required. The default function does nothing.

  =TM_quit=() is called once when EOF is reached on the
  telemetry stream. The default function does nothing.

=Returns

  TM_stream() returns zero on success, non-zero on error.

=SeeAlso

  =TM Functions=.

=End
*/

/*
=Name TM_read_stream(): Read a TM Stream
=Subject TM Functions
=Synopsis
#include "tm.h"
int TM_read_stream( void );

=Description

  TM_read_stream() opens the TM stream via =TM_open_stream=()
  unless it is already open, then calls TM_readfd() to read
  the stream. As such TM_read_stream() is a one-stop function
  for processing a TM stream.

=Returns

  Returns zero on success, non-zero on failure, but only if
  =nl_response= is set to a non-fatal level.

=SeeAlso

  =TM Functions=.

=End
*/

/*
=Name TM_readfd(): Low-level read from a TM Stream
=Subject TM Functions
=Synopsis
#include "tm.h"
int TM_readfd( void );

=Description

  Loops reading from TM_fd and passing the data on to
  =TM_stream=() until no data is available. At EOF,
  TM_readfd() calls TM_stream() with zero bytes to propogate the
  condition. TM_stream() in turn will call =TM_quit=().
  If the TM stream was opened in blocking mode, TM_readfd()
  will not return until the entire stream has been processed
  and TM_fd reports EOF. In non-blocking mode, TM_readfd will return
  as soon as the input buffer is drained.

=Returns

  TM_readfd() returns zero on success. It will return non-zero
  only if the TM stream TM_fd has not been opened and =nl_response=
  has been set to a non-fatal setting. In non-blocking modes,
  the calling application should use the =TM_quit=() function to
  identify the EOF condition.

=SeeAlso

  =TM Functions=.

=End
*/
