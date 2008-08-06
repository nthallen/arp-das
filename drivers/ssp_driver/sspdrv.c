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
#include <string.h> // For cmdee_init
#include <errno.h> // For cmdee_init
#include <fcntl.h> // For cmdee_init
#include <sys/select.h>
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

// define ssp_data_t structure in sspdrv.h
static ssp_data_t ssp_data;

static int is_eocmd( char c ) {
  return c == '\0' || isspace(c);
}

static void report_invalid( char *head ) {
  char *tail = head;
  while ( !is_eocmd(tail) ) ++tail;
  *tail = '\0';
  nl_error( 2, "Invalid command received: '%s'", head );
}

/** Read a command line from cmd_fd (cmd/SSPn)
 I will accept space-delimited combinations of the following:
 Commands:
   Allowed when not acquiring:
    EN Enable
    NS:xxxx N_Samples
    NA:xxxx N_Average (Pre-Adder)
    NC:xxxx N_Coadd
    NF:xx      Frequency Divisor
    NE:x 1-7 bit-mapped Specifies which channels are enabled

   Allowed anytime:
    DA Disable
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
  nb = read( cmd_fd, buf, CMDEE_BUFSIZE-2 );
  if ( nb == -1 )
    nl_error(3, "Error reading from cmd/SSP: %s", strerror(errno) );
  if ( nb == 0 ) {
    // EOF == Quit
    quit_received = 1;
    if ( udp_state == FD_IDLE ) return;
    strcpy(buf, "DA");
    nb = strlen(buf);
  }
  nl_assert(nb < CMDEE_BUFSIZE-1);
  // Put two NULs at the end of the buffer
  buf[nb] = buf[nb+1] = '\0';
  head = buf;
  while ( *head ) {
    while ( isspace(*head) ) ++head;
    tail = head;
    switch (*tail) {
      case 'E':
        if ( *++tail != 'N' || !is_eocmd(*++tail) ) {
          report_invalid(head);
          return;
        }
        *tail = '\0';
        if ( udp_state != FD_IDLE ) {
          nl_error( 2, "EN not valid: Already enabled" );
          return;
        }
        { int udp_port = udp_create();
          char udp_buf[20];
          snprintf(udp_buf, 20, "NP:%d", udp_port);
          tcp_queue(udp_buf);
          tcp_queue(head);
        }
        break;
      case 'D':
        if ( *++tail != 'A' || !is_eocmd(*++tail) ) {
          report_invalid(head);
          return;
        }
        *tail = '\0';
        udp_close();
        tcp_queue(head);
        break;
      case 'N':
      case 'A':
    }
    head = tail+1;
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
  mlf = mlf_init( 3, 60, 1, msg_hdr, "dat", mlf_config );
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
      udp_recv();
    if ( FD_ISSET(cmd_fd, &readfds) ) read_cmd( cmd_fd );
    if ( FD_ISSET(tm_data->fd, &writefds ) ) Col_send(tm_data);
    if ( FD_ISSET(tcp_fd, &readfds ) ) tcp_recv();
    if ( FD_ISSET(tcpfd, &writefds ) ) tcp_send();
  }
  // ### Add shutdown stuff
}
