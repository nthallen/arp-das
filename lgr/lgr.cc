#include <string.h>
#include <time.h>
#include <errno.h>
#include "DC.h"
#include "nortlib.h"
#include "nl_assert.h"
#include "oui.h"
#include "lgr.h"

void lgr_init( int argc, char **argv ) {
  int c;

  optind = OPTIND_RESET; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'N':
        DClgr::mlf_config = optarg;
        break;
      case '?':
        nl_error(3, "Unrecognized Option -%c", optopt);
    }
  }
}

char *DClgr::mlf_config;
unsigned int DClgr::file_limit = 4096*3;

DClgr::DClgr() : data_client( 4096, 0, 0 ) {
  mlf = mlf_init( 3, 60, 1, "LOG", "dat", mlf_config );
  ofp = NULL;
}

// Since we accept data from any frame, we need to copy the incoming
// frame definition into tm_info so data_client::process_init()
// will be happy.
void DClgr::process_init() {
  memcpy(&tm_info, &msg->body.init.tm, sizeof(tm_dac_t));
  data_client::process_init();
}

void DClgr::process_tstamp() {
  data_client::process_tstamp();
  // There should be room for the TS and at least one row
  // otherwise just move on to the next file.
  if (ofp == NULL ||
      nb_out + sizeof(tm_hdr_t) + sizeof(tstamp_t) +
      nbDataHdr + nbQrow > file_limit) {
    next_file();
  } else {
    write_tstamp();
  }
}

void DClgr::lgr_write(void *buf, int nb, char *where ) {
  if ( fwrite( buf, nb, 1, ofp ) < 1 )
    nl_error( 3, "Error %s: %s", where, strerror(errno) );
  fflush(ofp);
  nb_out += nb;
}

void DClgr::next_file() {
  if ( ofp != NULL && fclose(ofp) ) {
    nl_error( 2, "Error closing file: %s", strerror(errno) );
  }
  ofp = mlf_next_file(mlf);
  if ( ofp == NULL )
    nl_error( 3, "Unable to open output file" );
  nb_out = 0;
  write_tstamp();
}

void DClgr::write_tstamp() {
  nl_assert( ofp != NULL );
  static tm_hdr_t ts_hdr = { TMHDR_WORD, TMTYPE_TSTAMP };
  lgr_write(&ts_hdr, sizeof(tm_hdr_t), "writing tstamp hdr" );
  lgr_write( &tm_info.t_stmp, sizeof(tstamp_t), "writing tstamp" ); 
}

void DClgr::process_data_t3() {
  tm_data_t3_t *data = &msg->body.data3;
  int n_rows = data->n_rows;
  unsigned short mfctr = data->mfctr;
  unsigned char *wdata = &data->data[0];
  tm_msg_t wmsg;

  wmsg.hdr.tm_id = TMHDR_WORD;
  wmsg.hdr.tm_type = TMTYPE_DATA_T3;
  if ( ofp == NULL ) next_file();

  while (n_rows > 0) {
    int space_remaining = file_limit - nb_out;
    int rows_fit = ( space_remaining - nbDataHdr ) / nbQrow;
    if ( rows_fit < 1 && nb_out == sizeof(tm_hdr_t) + sizeof(tstamp_t) ) 
      rows_fit = 1;
    if ( rows_fit < 1 ) {
      next_file();
    } else {
      if ( rows_fit > n_rows )
	rows_fit = n_rows;
      wmsg.body.data3.n_rows = rows_fit;
      wmsg.body.data3.mfctr = mfctr;
      lgr_write( &wmsg, nbDataHdr, "writing data header" );
      lgr_write( wdata, nbQrow*rows_fit, "writing data" );
      n_rows -= rows_fit;
      mfctr += rows_fit;
      wdata += nbQrow*rows_fit;
      if ( n_rows ) next_file();
    }
  }
}

void DClgr::process_data() {
  switch ( output_tm_type ) {
    case TMTYPE_DATA_T1:
    case TMTYPE_DATA_T2:
      nl_error(3,"Data type not implemented" );
    case TMTYPE_DATA_T3:
      process_data_t3();
      break;
  }
}

void tminitfunc() {}

int main( int argc, char **argv ) {
  oui_init_options(argc, argv);
  DClgr DC;
  DC.operate();
  return 0;
}

