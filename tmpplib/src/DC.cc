/** \file DC.cc
 * Data Client Clases
 */
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "DC.h"
#include "nortlib.h"

unsigned int data_client::next_minor_frame;
unsigned int data_client::minf_row;
unsigned int data_client::majf_row;
char *data_client::srcnode = NULL;

void dc_set_srcnode(char *nodename) {
  data_client::srcnode = nodename;
}

void data_client::init(int bufsize_in, int non_block, char *srcfile) {
  bufsize = bufsize_in;
  bytes_read = 0;
  next_minor_frame = 0;
  majf_row = 0;
  minf_row = 0;
  dc_state = DC_STATE_HDR;
  toread = sizeof(tm_hdr_t);
  buf = new char[bufsize];
  tm_info_ready = false;
  dc_quit = false;
  if ( buf == 0)
    nl_error( 3, "Memory allocation failure in data_client::data_client");
  msg = (tm_msg_t *)buf;
  non_block = non_block ? O_NONBLOCK : 0;
  bfr_fd =
    srcfile ?
    tm_open_name( srcfile, srcnode, O_RDONLY | non_block ) :
    -1;
}

data_client::data_client(int bufsize_in, int non_block, char *srcfile) {
  init(bufsize_in, non_block, srcfile );
}

data_client::data_client(int bufsize_in, int fast, int non_block) {
  init(bufsize_in, non_block, tm_dev_name( fast ? "TM/DCf" : "TM/DCo" ));
}

int data_client::process_eof() {
  dc_quit = true;
  return 1;
}

void data_client::read() {
  int nb;
  do {
    nb = (bfr_fd == -1) ? 0 :
        ::read( bfr_fd, buf + bytes_read, bufsize-bytes_read);
    if ( nb == -1 ) {
      if ( errno == EAGAIN ) return; // must be non-blocking
      else nl_error( 1, "data_client::read error from read(): %s",
        strerror(errno));
    }
    if (nb <= 0) {
      bytes_read = 0;
      toread = sizeof(tm_hdr_t);
      if ( process_eof() ) return;
    }
    if ( dc_quit ) return; // possible if set from an outside command
  } while (nb == 0 );
  bytes_read += nb;
  if ( bytes_read >= toread )
    process_message();
}

/** This is the basic operate loop for a simple extraction
 *
 */
void data_client::operate() {
  tminitfunc();
  while ( !dc_quit ) {
    read();
  }
}

/* *
 * Internal function to establish input_tm_type.
 */
void data_client::init_tm_type() {
  if ( tmi(mfc_lsb) == 0 && tmi(mfc_msb) == 1
       && tm_info.nrowminf == 1 ) {
    input_tm_type = TMTYPE_DATA_T3;
    nbQrow = tmi(nbrow) - 4;
    nbDataHdr = 8;
  } else if ( tm_info.nrowminf == 1 ) {
    input_tm_type = TMTYPE_DATA_T1;
    nbQrow = tmi(nbrow);
    nbDataHdr = 6;
  } else {
    input_tm_type = TMTYPE_DATA_T2;
    nbQrow = tmi(nbrow);
    nbDataHdr = 10;
  }
  tm_info_ready = true;
}

void data_client::process_init() {
  if ( memcmp( &tm_info, &msg->body.init.tm, sizeof(tm_dac_t) ) )
    nl_error(3, "tm_dac differs");
  tm_info.nrowminf = msg->body.init.nrowminf;
  tm_info.max_rows = msg->body.init.max_rows;
  tm_info.t_stmp = msg->body.init.t_stmp;
  init_tm_type();
}

void data_client::process_tstamp() {
  tm_info.t_stmp = msg->body.ts;
}

void data_client::process_message() {
  while ( bytes_read >= toread ) {
    switch ( dc_state ) {
      case DC_STATE_HDR:
        switch ( msg->hdr.tm_type ) {
          case TMTYPE_INIT:
            if ( tm_info_ready )
              nl_error( 3, "Received redundant TMTYPE_INIT" );
            toread += sizeof(tm_info);
            break;
          case TMTYPE_TSTAMP:
            if ( !tm_info_ready )
              nl_error( 3, "Expected TMTYPE_INIT, received TMTYPE_TSTAMP" );
            toread += sizeof(tstamp_t);
            break;
          case TMTYPE_DATA_T1:
          case TMTYPE_DATA_T2:
          case TMTYPE_DATA_T3:
          case TMTYPE_DATA_T4:
            if ( !tm_info_ready )
              nl_error( 3, "Expected TMTYPE_INIT, received TMTYPE_DATA_Tn" );
            if ( msg->hdr.tm_type != input_tm_type )
              nl_error(3, "Invalid data type: %04X", msg->hdr.tm_type );
            toread = nbDataHdr + nbQrow * msg->body.data1.n_rows;
            break;
          default: nl_error( 3, "Invalid TMTYPE: %04X", msg->hdr.tm_type );
        }
        dc_state = DC_STATE_DATA;
        if ( toread > bufsize )
          nl_error( 3, "Record size %d exceeds allocated buffer size %d",
            toread, bufsize );
        break;
      case DC_STATE_DATA:
        switch ( msg->hdr.tm_type ) {
          case TMTYPE_INIT:
            process_init();
            break;
          case TMTYPE_TSTAMP:
            process_tstamp();
            break;
          case TMTYPE_DATA_T1:
          case TMTYPE_DATA_T2:
          case TMTYPE_DATA_T3:
          case TMTYPE_DATA_T4:
            process_data();
            break;
        }
        if ( bytes_read > toread ) {
          memmove(buf, buf+toread, bytes_read - toread);
          bytes_read -= toread;
        } else if ( bytes_read == toread ) {
          bytes_read = 0;
        }
        toread = sizeof(tm_hdr_t);
        dc_state = DC_STATE_HDR;
        break;
      default: nl_error( 4, "Invalid dc_state" );
    }
  }
}

void data_client::resize_buffer( int bufsize_in ) {
  delete buf;
  bufsize = bufsize_in;
  buf = new char[bufsize];
  if ( buf == 0)
    nl_error( 3,
       "Memory allocation failure in data_client::resize_buffer");
}

void data_client::load_tmdac(char *path) {
  ::load_tmdac(path);
  init_tm_type();
}
