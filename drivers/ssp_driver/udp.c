#include <stdlib.h>
#include "nortlib.h"
#include "nl_assert.h"
#include "sspint.h"

int udp_socket;
enum fdstate udp_state = FD_IDLE;

static long int scan_buf[SSP_CLIENT_BUF_LENGTH];
static long int scan0 = 6, scan1, scan5 = 0l;
static int raw_length, n_channels, scan_size;
static int cur_word, scan_serial_number, frag_hold, scan_OK;
static int cfgerr_reported;

int udp_create(void) {
  struct sockaddr_in servAddr;
  int port, rc;
  socklen_t servAddr_len = sizeof(servAddr);

  nl_assert( udp_state == FD_IDLE );

  /* First check to make sure the configuration is valid */
  if ( ssp_config.NS == 0 ) {
    ssp_config.NS = 1024;
    tcp_enqueue("NS:1024");
  }
  if ( ssp_config.NE == 0 ) {
    ssp_config.NE = 1;
    tcp_enqueue("NE:1");
  }
  switch ( ssp_config.NE ) {
    case 1: case 2: case 4: n_channels = 1; break;
    case 3: case 5: case 6: n_channels = 2; break;
    case 7: n_channels = 3; break;
    default:
      nl_error( 2, "Invalid NE configuration: %d", ssp_config.NE );
      return -1;
  }
  raw_length = (7 + ssp_config.NS*n_channels);
  scan_size = raw_length*sizeof(long);
  scan1 = (ssp_config.NS << 16) + n_channels;

  /* socket creation */
  udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket<0)
    nl_error( 3, "Cannot open UDP socket: %s", strerror(errno) );

  /* bind local server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(0);
  rc = bind (udp_socket, (struct sockaddr *) &servAddr,sizeof(servAddr));
  if(rc<0) nl_error( 3, "cannot bind port number %d \n",  0);
  rc = getsockname( udp_socket, (struct sockaddr *)&servAddr,
    &servAddr_len);
  port = ntohs(servAddr.sin_port);
  udp_state = FD_READ;
  cur_word = 0;
  scan_OK = 1;
  return port;
}

int udp_receive(long int *scan, size_t length ) {
  struct sockaddr_in cliAddr;
  int n;
  socklen_t cliLen;

  cliLen = sizeof(cliAddr);
  n = recvfrom(udp_socket, scan, length, 0, 
   (struct sockaddr *) &cliAddr, &cliLen);
  return n;
}

void udp_close(void) {
  if ( udp_state != FD_IDLE ) {
    close(udp_socket);
    udp_state = FD_IDLE;
  }
  ssp_config.NS = 0;
  ssp_config.NE = 0;
  ssp_config.NA = 0;
  ssp_config.NC = 0;
  ssp_config.NF = 0;
  ssp_config.NP = -1;
  cfgerr_reported = 0;
}

static void output_scan( long int *scan, mlf_def_t *mlf ) {
  FILE *ofp;
  ssp_scan_header_t *hdr = (ssp_scan_header_t *)scan;
  long int *idata = scan+hdr->NWordsHdr;
  float fdata[SSP_MAX_CHANNELS][SSP_MAX_SAMPLES];
  // time_t now;
  // static time_t last_rpt = 0;
  float divisor = 1/(hdr->NCoadd * (float)(hdr->NAvg+1));
  int my_scan_length = hdr->NSamples * hdr->NChannels;

  // scan is guaranteed to be raw_length words long. Want to verify that NSamples*NChannels + NWordsHdr + 1 == raw_length
  if ( hdr->NWordsHdr != scan0 ) {
    nl_error( 2, "NWordsHdr(%u) != %u", hdr->NWordsHdr, scan0 );
    return;
  }
  if ( hdr->FormatVersion > 1 ) {
    nl_error( 2, "Unsupported FormatVersion: %u", hdr->FormatVersion );
    return;
  }
  if ( my_scan_length + 7 != raw_length ) {
    nl_error( 2, "Header reports NS:%u NC:%u -- raw_length is %d",
      hdr->NSamples, hdr->NChannels, raw_length );
    return;
  }
  if (ssp_config.LE) {
    int i, j, nc = hdr->NChannels;
    for ( j = 0; j < hdr->NChannels; ++j ) {
      long int *id = &idata[j];
      for ( i = 0; i < hdr->NSamples; ++i ) {
        fdata[j][i] = (*id) * divisor;
        id += nc;
      }
    }
    ofp = mlf_next_file(mlf);
    fwrite(hdr, sizeof(ssp_scan_header_t), 1, ofp);
    fwrite(&scan[raw_length-1], sizeof(long int), 1, ofp);
    { int NCh = hdr->NChannels, j;
      for ( j = 0; j <= NCh; j++ ) {
        fwrite( fdata[j], sizeof(float), hdr->NSamples, ofp);
      }
    }
    fclose(ofp);
  }

  ssp_data.index = mlf->index;
  ssp_data.Flags |= (unsigned short)(scan[raw_length-1]);
  ssp_data.Total_Skip += hdr->NSkL + hdr->NSkP;
  ssp_data.ScanNum = hdr->ScanNum;
  if ( hdr->FormatVersion > 0 ) {
    ssp_data.T_FPGA = hdr->T_FPGA;
    ssp_data.T_HtSink = hdr->T_HtSink;
  }
  
  // Perform some sanity checks on the inbound scan
  if ( (scan[1] & 0xFFFF00FF) != scan1 )
    nl_error( 1, "%lu: scan[1] = %08lX (not %08lX)\n", mlf->index, scan[1], scan1 );
  if ( hdr->FormatVersion == 0 && scan[5] != scan5 )
    nl_error( 1, "%lu: scan[5] = %08lX (not %08lX)\n", mlf->index, scan[5], scan5 );
}

void udp_read(mlf_def_t *mlf) {
  int n = udp_receive(scan_buf+cur_word,
    cur_word ? MAX_UDP_PAYLOAD : SSP_MAX_SCAN_SIZE);
  if ( n < 0 )
    nl_error( 2, "Error from udp_receive: %d", errno );
  else if ( cur_word == 0 && !(*scan_buf & SSP_FRAG_FLAG) ) {
    if ( n == scan_size ) output_scan(scan_buf, mlf);
    else nl_error( 2, "Expected %d bytes, received %d", scan_size, n );
  } else if ( !( scan_buf[cur_word] & SSP_FRAG_FLAG ) ) {
    nl_error( 2, "Expected scan fragment" );
  } else {
    int frag_hdr = scan_buf[cur_word];
    int frag_offset = frag_hdr & 0xFFFFL;
    int frag_sn;
    if ( frag_offset != cur_word ) {
      if ( frag_offset == 0 ) {
        memmove( scan_buf, scan_buf+cur_word, n );
        if ( scan_OK ) nl_error( 2, "Lost end of scan." );
        cur_word = 0;
        scan_OK = 1;
      } else if ( scan_OK ) {
        nl_error( 2, "Lost fragment" );
        scan_OK = 0;
      }
    }
    frag_sn = frag_hdr & 0x3FFF0000L;
    if ( cur_word == 0 ) scan_serial_number = frag_sn;
    else {
      scan_buf[cur_word] = frag_hold;
      if ( scan_OK && scan_serial_number != frag_sn ) {
        scan_OK = 0;
        nl_error( 2, "Lost data: SN skip" );
      }
    }
    cur_word = frag_offset + (n/sizeof(long)) - 1;
    if ( frag_hdr & SSP_LAST_FRAG_FLAG ) {
      if ( scan_OK ) {
        if ( cur_word == raw_length )
          output_scan( scan_buf+1, mlf );
        else nl_error( 2, "Scan length error: expected %d words, received %d",
          raw_length, cur_word );
      }
      cur_word = 0;
      scan_OK = 1;
    } else {
      frag_hold = scan_buf[cur_word];
    }
    if ( cur_word + SSP_MAX_FRAG_LENGTH > SSP_CLIENT_BUF_LENGTH ) {
      nl_error( 2, "Bad fragment offset: %d(%d)", frag_offset, n );
      cur_word = 0;
      scan_OK = 1;
    }
  }
}
