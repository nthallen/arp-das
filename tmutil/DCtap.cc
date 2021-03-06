#include <string.h>
#include <time.h>
#include "DC.h"
#include "DCtap.h"
#include "nortlib.h"
#include "oui.h"

class DCtap : public data_client {
  public:
    DCtap(char *srcfile);
    DCtap();
    static char *dacfile;
    static char *srcfile;
  protected:
    void process_data_t1();
    void process_data_t2();
    void process_data_t3();
    void process_data();
    void process_init();
    void process_tstamp();
};

char *DCtap::dacfile;
char *DCtap::srcfile;
DCtap::DCtap() : data_client( 4096, 1, 0 ) {}
DCtap::DCtap( char *srcfile ) : data_client( 4096, 0, srcfile ) {}

void DCtap::process_init() {
  memcpy(&tm_info, &msg->body.init.tm, sizeof(tm_dac_t));
  data_client::process_init();
  nl_error( 0, "process_init()" );
}

void DCtap::process_tstamp() {
  struct tm *tm = gmtime( &msg->body.ts.secs );
  char ttext[30];
  asctime_r(tm, ttext);
  int tlen = strlen(ttext);
  ttext[tlen-1] = '\0';
  nl_error( 0, "process_tstamp(%s, %5d)", ttext, msg->body.ts.mfc_num );
  data_client::process_tstamp();
}

void DCtap::process_data_t3() {
  tm_data_t3_t *data = &msg->body.data3;
  int n_rows = data->n_rows;
  unsigned short mfctr = data->mfctr;
  int i, k;

  nl_error(0, "MF: %5u  %d rows", mfctr, n_rows);
  for (i = k = 0; i < n_rows; i++ ) {
    char buf[64];
    int j = 0, p = 0;
    p = snprintf(buf, 64, "  %5u:", mfctr+i);
    for ( j = 0; j < nbQrow; ++j ) {
      if ( j > 0 && j%16 == 0 ) {
	nl_error( 0, "%s", buf );
	p = snprintf( buf, 64, "        " );
      }
      p += snprintf( buf+p, 64-p, " %02X", data->data[k++] );
    }
    nl_error( 0, "%s", buf );
  }
}

void DCtap::process_data() {
  //data_client::process_data();
  //nl_error( 0, "DCtap::process_data()" );
  switch ( input_tm_type ) {
    case TMTYPE_DATA_T1:
    case TMTYPE_DATA_T2:
      nl_error(3,"Data type not implemented" );
    case TMTYPE_DATA_T3:
      process_data_t3();
      break;
  }
}

void data_client::process_data() {
  nl_error( 0, "data_client::process_data()" );
}

void tminitfunc() {}

int main( int argc, char **argv ) {
  oui_init_options(argc, argv);
  nl_error( 0, "Startup" );
  DCtap *DC;
  if ( DCtap::srcfile ) {
    DC = new DCtap( DCtap::srcfile );
  } else {
    DC = new DCtap;
  }
  if ( DCtap::dacfile )
    DC->load_tmdac(DCtap::dacfile);
  DC->operate();
  nl_error( 0, "Shutdown" );
  return 0;
}

void DCtap_init_options(int argc, char **argv) {
  int optltr;

  optind = OPTIND_RESET;
  opterr = 0;
  while ((optltr = getopt(argc, argv, opt_string)) != -1) {
    switch (optltr) {
      case 'S': DCtap::srcfile = optarg; break;
      case 'T': DCtap::dacfile = optarg; break;
      case '?':
	nl_error(3, "Unrecognized Option -%c", optopt);
      default:
	break;
    }
  }
}
