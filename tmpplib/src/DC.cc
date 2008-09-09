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
  toread = sizeof(tm_hdr_t);
  buf = new char[bufsize];
  tm_info_ready = false;
  quit = false;
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

void data_client::process_eof() {
  quit = true;
}

void data_client::read() {
  int nb;
  do {
    nb = (bfr_fd == -1) ? 0 :
	::read( bfr_fd, buf + bytes_read, bufsize-bytes_read);
    if (nb == 0) process_eof();
    if (quit) return;
  } while (nb == 0 );
  if ( nb == -1 ) {
    if (errno == EAGAIN) return; // must be non-blocking
    else nl_error( 4, "data_client::read error from read(): %s",
      strerror(errno) );
  }
  bytes_read += nb;
  if ( bytes_read >= toread )
    process_message();
}

/** This is the basic operate loop for a simple extraction
 *
 */
void data_client::operate() {
  tminitfunc();
  nl_error( 0, "Startup" );
  while ( !quit ) {
    read();
  }
  nl_error( 0, "Shutdown" );
}

void data_client::process_init() {
  if ( memcmp( &tm_info, &msg->body.init.tm, sizeof(tm_dac_t) ) )
    nl_error(3, "tm_dac differs");
  tm_info.nrowminf = msg->body.init.nrowminf;
  tm_info.max_rows = msg->body.init.max_rows;
  tm_info.t_stmp = msg->body.init.t_stmp;
  if ( tmi(mfc_lsb) == 0 && tmi(mfc_msb) == 1
       && tm_info.nrowminf == 1 ) {
    output_tm_type = TMTYPE_DATA_T3;
    nbQrow = tmi(nbrow) - 4;
    nbDataHdr = 8;
  } else if ( tm_info.nrowminf == 1 ) {
    output_tm_type = TMTYPE_DATA_T1;
    nbQrow = tmi(nbrow);
    nbDataHdr = 6;
  } else {
    output_tm_type = TMTYPE_DATA_T2;
    nbQrow = tmi(nbrow);
    nbDataHdr = 10;
  }
  tm_info_ready = true;
}

void data_client::process_tstamp() {
  tm_info.t_stmp = msg->body.ts;
  //nl_error( 0, "data_client::process_tstamp" );
}

void data_client::process_message() {
  while ( bytes_read >= sizeof(tm_hdr_t) ) {
    if ( msg->hdr.tm_id != TMHDR_WORD )
      nl_error( 3, "Invalid data from TMbfr" );
    if ( !tm_info_ready ) {
      if ( msg->hdr.tm_type != TMTYPE_INIT )
        nl_error( 3, "Expected TMTYPE_INIT" );
      toread = sizeof(tm_hdr_t)+sizeof(tm_info_t);
      if ( bytes_read >= toread )
        process_init();
    } else {
      switch ( msg->hdr.tm_type ) {
        case TMTYPE_INIT: nl_error( 3, "Unexpected TMTYPE_INIT" ); break;
        case TMTYPE_TSTAMP:
	  toread += sizeof(tstamp_t);
	  if ( bytes_read >= toread )
	    process_tstamp();
	  break;
        case TMTYPE_DATA_T1:
        case TMTYPE_DATA_T2:
        case TMTYPE_DATA_T3:
        case TMTYPE_DATA_T4:
          if ( msg->hdr.tm_type != output_tm_type )
            nl_error(3, "Invalid data type: %04X", msg->hdr.tm_type );
          toread = nbDataHdr + nbQrow * msg->body.data1.n_rows;
          if ( bytes_read >= toread ) process_data();
          break;
        default: nl_error( 3, "Invalid TMTYPE: %04X", msg->hdr.tm_type );
      }
    }
    if ( bytes_read > toread ) {
      memmove(buf, buf+toread, bytes_read - toread);
      bytes_read -= toread;
      toread = sizeof(tm_hdr_t);
    } else if ( bytes_read == toread ) {
      bytes_read = 0;
      toread = sizeof(tm_hdr_t);
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

FILE *data_client::open_path( char *path, char *fname ) {
  char filename[PATH_MAX];
  if ( snprintf( filename, PATH_MAX, "%s/%s", path, fname )
	 >= PATH_MAX )
    nl_error( 3, "Pathname overflow for file '%s'", fname );
  FILE *tmd = fopen( filename, "r" );
  return tmd;
}

void data_client::load_tmdac( char *path ) {
  if ( path == NULL || path[0] == '\0' ) path = ".";
  FILE *dacfile = open_path( path, "tm.dac" );
  if ( dacfile == NULL ) {
    char version[40];
    char dacpath[80];

    version[0] = '\0';
    FILE *ver = open_path( path, "VERSION" );
    if ( ver != NULL ) {
      int len;
      if ( fgets( version, 40, ver ) == NULL )
	nl_error(3,"Error reading VERSION: %s",
	  strerror(errno));
      len = strlen(version);
      while ( len > 0 && isspace(version[len-1]) )
	version[--len] = '\0';
      if ( len == 0 )
	nl_error( 1, "VERSION was empty: assuming 1.0" );
    } else {
      nl_error( 1, "VERSION not found: assuming 1.0" );
    }
    if ( version[0] == '\0' ) strcpy( version, "1.0" );
    snprintf( dacpath, 80, "bin/%s/tm.dac" );
    dacfile = open_path( path, dacpath );
  }
  if ( dacfile == NULL ) nl_error( 3, "Unable to locate tm.dac" );
  if ( fread(&tm_info.tm, sizeof(tm_dac_t), 1, dacfile) != 1 )
    nl_error( 3, "Error reading tm.dac" );
  fclose(dacfile);
}
