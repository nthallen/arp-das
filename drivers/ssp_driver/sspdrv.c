/**
 sspdrv.c Driver framework for
 Harvard Anderson Group Scalable Signal Processing board (SSP)
 
 Options:
  -specify board IP
  -specify command channel
  -specify logging hierarchy
  
 Strategy:
 - Open tcp connection to ssp
 - Open read from cmd/SSP (different channel for each board)
 - Open write to DG/data/SSP (different channel for each board)

 Accept commands and queue requests to the board.
 Open UDP connection on start and add to the select
 
 Channels:
   cmd_fd: cmd/SSPn: always reading: READ
   tm_data->fd: DG/data/SSPn: always writing: may be synchronized: WRITE
   tcp_fd: ssp TCP: alternately reading and writing: IDLE,WRITE,READ
   udp_fd: ssp UDP: open and closed, always reading: IDLE,READ
 */
#include <fcntl.h> // For cmdee_init
#include <sys/select.h>
#include <stdlib.h>
#include <ctype.h>
#include "oui.h"
#include "nortlib.h"
#include "nl_assert.h"
#include "tm.h"
#include "collect.h"
#include "sspint.h"

static int board_id = 0;
static char *mlf_config = NULL;
#define MSG_HDR_SIZE 8
static char msg_hdr[MSG_HDR_SIZE];
static int quit_received = 0;
ssp_config_t ssp_config;
ssp_data_t ssp_data;

void sspdrv_init( int argc, char **argv ) {
  int c;

  optind = OPTIND_RESET; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'b':
        board_id = atoi(optarg);
        if ( board_id < 0 || board_id > 54 )
          nl_error( 3, "Invalid board_id: %d", board_id );
        break;
      case 'N':
        mlf_config = optarg;
        break;
      case '?':
        nl_error(3, "Unrecognized Option -%c", optopt);
    }
  }
  snprintf(msg_hdr, MSG_HDR_SIZE, "SSP%d", board_id );
}

/**
 * Initializes a read-only connection to the command server. This should be moved into
 * tmlib
 */
#define CMDEE_BUFSIZE 160
static int cmdee_init( char *cmd_node ) {
  char buf[CMDEE_BUFSIZE];
  int fd;

  nl_assert(cmd_node != NULL);
  snprintf(buf, CMDEE_BUFSIZE, "cmd/%s", cmd_node );
  fd = open( tm_dev_name(buf), O_RDONLY );
  if ( fd == -1 ) {
    if (nl_response)
      nl_error( nl_response, "Unable to open tm device %s: %s",
        buf, strerror(errno) );
  }
  return fd;
}

static int is_eocmd( char c ) {
  return c == '\0' || isspace(c);
}

static void report_invalid( char *head ) {
  char *tail = head;
  while ( !is_eocmd(*tail) ) ++tail;
  *tail = '\0';
  nl_error( 2, "Invalid command received: '%s'", head );
}

/** Read a command line from cmd_fd (cmd/SSPn)
 I will accept space-delimited combinations of the following:
 Commands:
   Allowed when not acquiring:
    -EN Enable
    NS:xxxx N_Samples
    NA:xxxx N_Average (Pre-Adder)
    NC:xxxx N_Coadd
    NF:xx      Frequency Divisor
    NE:x 1-7 bit-mapped Specifies which channels are enabled

   Allowed anytime:
    -DA Disable
    NU:[-]xxxxx Level Trigger Rising
    ND:[-]xxxxx Level Trigger Falling
    NT:x (0-3) Specifies the trigger source
    AE Autotrig Enable
    AD Autotrig Disable

   Never allowed:
    EX Quit: don't do it!
    NP:xxxxx UDP Port Number (I specify this)
 
  As before, we will accept triggering commands anytime and other command only when not acquiring.
 */

void read_cmd( int cmd_fd ) {
  char buf[CMDEE_BUFSIZE], *head, *tail;
  int nb;
  nb = read( cmd_fd, buf, CMDEE_BUFSIZE-1 );
  if ( nb == -1 )
    nl_error(3, "Error reading from cmd/SSP: %s", strerror(errno) );
  if ( nb == 0 ) {
    // EOF == Quit
    quit_received = 1;
    if ( udp_state == FD_IDLE ) return;
    strcpy(buf, "DA");
    nb = strlen(buf);
  }
  nl_assert(nb < CMDEE_BUFSIZE);
  buf[nb] = '\0';
  head = buf;
  while ( *head ) {
    while ( isspace(*head) ) ++head;
    tail = head;
    switch (*head) {
      case 'E':
        if ( *++tail != 'N' || !is_eocmd(*++tail) ) {
          report_invalid(head);
          return;
        }
        if ( udp_state != FD_IDLE ) {
          nl_error( 2, "EN not valid: Already enabled" );
          return;
        }
        { ssp_config.NP = udp_create();
          char udp_buf[20];
          snprintf(udp_buf, 20, "NP:%d", ssp_config.NP);
          tcp_enqueue(udp_buf);
        }
        break;
      case 'D':
        if ( *++tail != 'A' || !is_eocmd(*++tail) ) {
          report_invalid(head);
          return;
        }
        udp_close();
        break;
      case 'A':
        switch (*++tail) {
          case 'E':
          case 'D':
            if ( is_eocmd(*++tail) ) break;
            // else fall through
          default:
            report_invalid(head);
            return;
        }
        break;
      case 'N':
        if ( !is_eocmd(*++tail) && *++tail == ':' ) {
          char *num = ++tail;
          unsigned int newval;
          if ( *tail == '-' ) ++tail;
          if ( !isdigit(*tail) ) {
            report_invalid(head);
            return;
          }
          while ( isdigit(*tail) ) ++tail;
          if ( !is_eocmd(*tail) ) {
            report_invalid(head);
            return;
          }
          newval = atoi(num);
          switch (head[1]) {
            case 'S':
            case 'A':
            case 'C':
            case 'F':
            case 'E':
              if ( udp_state != FD_IDLE ) {
                *tail = '\0';
                nl_error( 2, "Command invalid while SSP is enabled: '%s'", head );
                return;
              }
              break;
            case 'U':
            case 'D':
            case 'T':
              break;
            default:
              report_invalid(head);
              return;
          }
          switch (head[1]) {
            case 'S': ssp_config.NS = newval; break;
            case 'A': ssp_config.NA = newval; break;
            case 'C': ssp_config.NC = newval; break;
            case 'F': ssp_config.NF = newval; break;
            case 'E': ssp_config.NE = newval; break;
            case 'U': break;
            case 'D': break;
            case 'T': break;
            default:
              nl_error(4,"Impossible");
          }
          break;
        } else {
          report_invalid(head);
          return;
        }
      default:
        report_invalid(head);
        return;
    }
    { char save_char = *tail;
      *tail = '\0';
      tcp_enqueue(head);
      *tail = save_char;
      head = tail;
    }
  }
}

int main( int argc, char **argv ) {
  mlf_def_t *mlf;
  int cmd_fd, tcp_fd = -1, udp_fd = -1;
  int non_udp_width, udp_width;
  send_id tm_data;
  fd_set readfds, writefds;
  
  oui_init_options(argc, argv);
  // ### initialize connection to memo
  udp_close(); // Initialize ssp_config and udp_state
  mlf = mlf_init( 3, 60, 1, msg_hdr, "dat", mlf_config );
  ssp_data.index = mlf->index;
  ssp_data.ScanNum = 0;
  ssp_data.Flags = 0;
  ssp_data.Total_Skip = 0;
  cmd_fd = cmdee_init( msg_hdr );
  tm_data = Col_send_init(msg_hdr, &ssp_data, sizeof(ssp_data), 0);
  tcp_fd = tcp_create(board_id);
  non_udp_width = cmd_fd + 1;
  if ( tm_data->fd >= non_udp_width )
    non_udp_width = tm_data->fd + 1;
  if ( tcp_fd >= non_udp_width )
    non_udp_width = tcp_fd + 1;
  // initialize select fd sets
  while (!quit_received) {
    int n_ready;
    FD_ZERO( &readfds );
    FD_ZERO( &writefds );
    FD_SET( cmd_fd, &readfds );
    FD_SET( tm_data->fd, &writefds );
    switch ( tcp_state ) {
      case FD_IDLE:
        break;
      case FD_WRITE:
        FD_SET(tcp_fd, &writefds );
        break;
      case FD_READ:
        FD_SET(tcp_fd, &readfds );
        break;
      default: nl_error(4, "Bad case for tcp_state" );
    }
    switch ( udp_state ) {
      case FD_IDLE:
        udp_width = non_udp_width;
        break;
      case FD_READ:
        FD_SET(udp_fd, &readfds );
        udp_width = udp_fd >= non_udp_width ? udp_fd + 1 : non_udp_width;
        break;
      default: nl_error(4, "Bad case for udp_state" );
    }
    n_ready = select( udp_width, &readfds, &writefds, NULL, NULL );
    if ( n_ready == -1 ) nl_error( 3, "Error from select: %s", strerror(errno));
    if ( n_ready == 0 ) nl_error( 3, "select() returned zero" );
    if ( udp_state == FD_READ && FD_ISSET( udp_fd, &readfds ) )
      udp_read(mlf);
    if ( FD_ISSET(cmd_fd, &readfds) ) read_cmd( cmd_fd );
    if ( FD_ISSET(tm_data->fd, &writefds ) ) {
      Col_send(tm_data);
      ssp_data.Flags &= ~SSP_OVF_MASK;
    }
    if ( FD_ISSET(tcp_fd, &readfds ) ) tcp_recv();
    if ( FD_ISSET(tcp_fd, &writefds ) ) tcp_send();
  }
  // ### Add shutdown stuff
  return 0;
}
