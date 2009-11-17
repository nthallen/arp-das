#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unix.h>
#include <ctype.h>

#include "oui.h"
#include "nortlib.h"
#include "collect.h"
#include "specq.h"
#include "tm.h"
#include "nl_assert.h"

static int Host_socket;
static char *HostName = "mattson";
static int specq_host_init( char *RemHost );
static void main_loop( send_id tmid, int cmd_fd );
static void read_command(int cmd_fd, send_id tmid);
static int report_error( char *txt );
static int Send_to_spec( char *fmt, char *txt );
static int tmreadline( int socket, int loop );

static specq_t SpecQ;
static int busy;
static int quitting;
// busy values are:
// 0: read for commands. No outstanding communication with specd
// 1: data has been written to specd and we're waiting for the reply
//    Only certain immediate responses will be entertained.
// quitting values are:
// 0: not quitting
// 1: A quit request came in while busy, and has not been forwarded to specd
// 2: A quit request has been forwarded to specd; waiting for reply

int main(int argc, char **argv ) {
  send_id tmid;
  int cmd_fd;
  oui_init_options(argc, argv);
  nl_error( 0, "Initializing" );

  // Initialize connection to telemetry
  tmid = Col_send_init("SpecQ", &SpecQ, sizeof(SpecQ), 0);
  SpecQ.status = 202;
  Col_send(tmid);

  // Initialize connection to command server
  set_response(2);
  cmd_fd = ci_cmdee_init("specq");
  if (cmd_fd >= 0 && specq_host_init(HostName) == 0) {
    SpecQ.status = 200;
    Col_send(tmid);
    nl_error( 0, "Initialized" );
    main_loop(tmid, cmd_fd);
  }
  SpecQ.status = 410; // Gone
  Col_send(tmid);
  // ### Close all connections
  nl_error( 0, "Terminated" );
  return 0;
}

static void main_loop( send_id tmid, int cmd_fd ) {
  int width = ( cmd_fd > Host_socket ? cmd_fd : Host_socket ) + 1;
  for(;;) {
    fd_set read_fds;
    int rv;
    
    FD_ZERO(&read_fds);
    FD_SET(cmd_fd, &read_fds);
    FD_SET(Host_socket, &read_fds);
    rv = select( width, &read_fds, NULL, NULL, NULL );
    if ( rv == -1 ) {
      report_error("From select()");
      return;
    } else if ( rv < 1 || rv > 2 ) {
      nl_error( 2, "select() returned %d" );
      return;
    }
    if (FD_ISSET(cmd_fd, &read_fds)) read_command(cmd_fd, tmid);
    if (FD_ISSET(Host_socket, &read_fds)) {
      rv = tmreadline(Host_socket, 0);
      if ( rv == 0 ) return; // error already reported
      SpecQ.status = rv;
      Col_send(tmid);
    }
    switch ( quitting ) {
      case 0: break;
      case 1:
        if ( !busy ) {
          rv = Send_to_spec( "%s\n", "exit" );
          SpecQ.status = rv;
          Col_send(tmid);
          if (rv != 202) return;
          quitting = 2;
        }
        break;
      case 2:
        if (!busy) return;
        break;
      default:
        nl_error( 4, "Invalid quitting code: %d", quitting );
    }
  }
}

static void read_command(int cmd_fd, send_id tmid) {
  char buf[MAX_PROTOCOL_LINE+1];
  char scan[MAX_PROTOCOL_LINE];
  int rv;
  
  rv = read(cmd_fd, buf, MAX_PROTOCOL_LINE);
  if ( rv < 0 ) {
    nl_error( 2, "Error %d (%s) reading from command server",
      errno, strerror(errno) );
  } else if ( rv == 0 ) {
    quitting = 1;
  } else if ( rv > MAX_PROTOCOL_LINE ) {
    nl_error( 4, "rv > MAX_PROTOCOL_LINE" );
  } else if ( busy ) {
    nl_error( 2, "Busy: ignoring command" );
  } else {
    int cmd;
    switch (buf[0]) {
      case 'R':
        switch (buf[1]) {
          case 'E': cmd = SPECQ_N_RESET; break;
          case 'N': cmd = SPECQ_N_RESET_SCAN; break;
          case 'S': cmd = SPECQ_N_RESET_STATUS; break;
          default: break;
        }
        break;
      case 'E':
        if (buf[1] == 'X') cmd = SPECQ_N_EXIT; break;
      case 'C':
        if (buf[1] == 'K') cmd = SPECQ_N_CHECK; break;
      case 'S':
        if (buf[1] == 'C') cmd = SPECQ_N_SCAN; break;
      default: break;
    }
    switch (cmd) {
      case SPECQ_N_EXIT:
        quitting = 1;
        break;
      case SPECQ_N_RESET:
        SpecQ.status = Send_to_spec( "%s\n", "reset" );
        if ( SpecQ.status/100 != 2 )
              nl_error( 2, "Reset returned %d", SpecQ.status );
        break;
      case SPECQ_N_CHECK:
        SpecQ.status = Send_to_spec( "%s\n", "check" );
        if ( SpecQ.status/100 != 2 )
              nl_error( 2, "Check returned %d", SpecQ.status );
        break;
      case SPECQ_N_SCAN:
        buf[rv] = '\0';
        snprintf(scan, MAX_PROTOCOL_LINE, &buf[3], ++SpecQ.scannum );
        SpecQ.status = Send_to_spec( "scan %s\n", scan );
        if ( SpecQ.status/100 != 2 )
          nl_error( 2, "Scan returned %d", SpecQ.status );
        break;
      case SPECQ_N_RESET_SCAN:
        SpecQ.scannum = atoi(&buf[3]);
        break;
      case SPECQ_N_RESET_STATUS:
        SpecQ.status = 0;
        break;
      default:
        nl_error( 2, "Invalid command" );
        break;
    }
    Col_send(tmid);
  }
}

/* Prints an error message and returns 1 */
static int report_error( char *txt ) {
  nl_error( 2, "%s(%d) %s", strerror(errno), errno, txt );
  return 1;
}

// Returns zero on success
static int specq_host_init( char *RemHost ) {
  struct servent *service;
  struct hostent *host;
  struct sockaddr_in server;
  int status;
  unsigned short Port;
  
  service = getservbyname( "specd", "tcp" );
  if ( service == NULL ) {
    nl_error( 2, "'specd' service not defined in /etc/services" );
    return 1;
  }
  Port = service->s_port;

  host = gethostbyname( RemHost );
  if ( host == NULL ) {
    char *errtxt;
    extern int h_errno;
    switch ( h_errno ) {
      case HOST_NOT_FOUND: errtxt = "Host Not Found"; break;
      case TRY_AGAIN: errtxt = "Try Again"; break;
      case NO_RECOVERY: errtxt = "No Recovery"; break;
      case NO_DATA: errtxt = "No Data"; break;
      default: errtxt = "Unknown"; break;
    }
    nl_error( 2, "\"%s\" error from gethostbyname", errtxt );
    return 1;
  }

  nl_error( -2, "Child: host_init getting there" );  
  Host_socket = socket( AF_INET, SOCK_STREAM, 0);
  if ( Host_socket == -1 ) return report_error( "from socket()" );
  server.sin_len = 0;
  server.sin_family = AF_INET;
  server.sin_port = Port;
  memcpy( &server.sin_addr, host->h_addr, host->h_length );
  status = connect( Host_socket, (struct sockaddr *)&server, sizeof(server) );
  if ( status == -1 ) return report_error( "from connect()" );
  status = Send_to_spec( "%s\n", "open specd!" );
  if (status/100 == 2)
    status = tmreadline( Host_socket, 1 );
  if (status/100 != 2) {
    nl_error( 2, "Open specd returned %d", status );
    return 1;
  }
  status = Send_to_spec( "%s\n", "check" );
  if (status/100 != 2) {
    nl_error( 2, "check failed after open specd" );
    return 1;
  }
  // We'll stay busy and handle this response in the main loop
  return 0;
}

static int Send_to_spec( char *fmt, char *txt ) {
  int rv;
  char buf[MAX_PROTOCOL_LINE+1];

  nl_assert( busy == 0 );
  if ( snprintf( buf, MAX_PROTOCOL_LINE+1, fmt, txt ) ==
        MAX_PROTOCOL_LINE ) {
    nl_error( 2, "Requested command too long for transmission" );
    return 400;
  }
  rv = write( Host_socket, buf, strlen(buf) );
  if ( rv == -1 ) {
    report_error( "writing to socket" );
    return 404; /* Don't know what to do! */
  }
  busy = 1;
  return 202;
}

/* This version reads a line, expecting a numeric status
   code as the leading characters. Returns 0 on error.
*/
static int tmreadline( int socket, int loop ) {
  static char buf[MAX_PROTOCOL_LINE], *bfr;
  static size_t nbytes = 0, tb = 0;
  size_t nb;

  if ( !busy ) nl_error( 1, "Unexpected input from specd" );
  if (nbytes == 0 ) {
    nbytes = MAX_PROTOCOL_LINE-1;
    bfr = buf;
    tb = 0;
  }
  while ( nbytes > 0 ) {
    nb = read( socket, bfr, nbytes );
    if (nb == -1) {
      nl_error( 2, "Read returned error: %d", errno );
      return 0;
    } else if ( nb <= 0 || nb > nbytes ) {
      nl_error( 2, "Read returned %d, expected %d", nb, nbytes );
      return 0;
    } else {
      tb += nb;
      buf[tb] = '\0';
      /* Did we get a complete line? */
      if ( buf[tb-1] == '\n' || buf[tb-1] == '\r' ) {
        if ( isdigit(buf[0]) ) {
          int rv = 0;
          for ( bfr = buf; isdigit(*bfr); bfr++ )
            rv = rv*10 + *bfr - '0';
          busy = 0;
          return rv;
        } else {
          nl_error( 2, "Expected numeric code, got: '%s'", buf );
          return 0;
        }
      }
    }
    /* We didn't get a complete line, so we'll wait for more */
    nbytes -= nb;
    bfr = (void *)(((char *)bfr) + nb);
    if ( !loop) return 202;
  }
  nl_error( 2, "Line too long from specd" );
  return 0;
}

void specq_opt_init( int argc, char **argv ) {
  int c;

  optind = 0; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt( argc, argv, opt_string )) != -1) {
    switch ( c ) {
      case 'H':
        HostName = optarg;
        break;
      case '?':
        nl_error(3, "Unrecognized Option -%c", optopt);
      default:
        /* Another init routine can handle other options */
        break;
    }
  }
}
