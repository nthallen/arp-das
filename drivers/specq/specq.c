/* specq.c

QNX Queuing program for spectrometers

This program will fork(), with the parent receiving and
queuing QNX requests and the child talking to #SpecDaemon via a
socket and to the parent via QNX messages.

The parent will run in a continuous Receive() loop servicing
requests from client(s) and from the child. It will enqueue
requests from the client(s) and Reply() them to the child.

The child will establish a connection with the remote
server and then Send() to the parent for instructions.
The Reply() will include command instructions, which the
child will relay as necessary to the remote server. When
the command is completed and the remote server returns
the command status, the child will Send() the status back
to the parent, which will notify the appropriate authorities.

Spec Queue Commands
EX exit:                   Sends exit command, waits for status, then closes socket
CK check:                  Sends check command
RE reset:                  Sends reset command
SC.* scan *:               Sends scan command and increments scan number
RN\d+ reset scannumber to n: Resets scan number
RS reset status to 0:      Resets status

Spec Queue will be designed to avoid needing to know the exact
syntax required by the spectrometer to which it is connected. The
scan commands it receives will be mostly opaque, but should
contain one printf-style sequence for interpolation of the scan
number. Scan resolution, number of scans and any other details as
yet unimagined will be embedded in the scan command by the
client, and Spec Queue will not need to parse them.

Queuing of commands is essential, even though we don't plan to
request more than one scan at a time because we don't want the
Spec Queue Client programs to block indefinitely, but this does
mean that status cannot be returned directly to the client, and
will have to be reported to telemetry instead. This will be done
through da_cache using two words:

 ScanNum 	 Current Scan Number
ScanStatus 	Current Scan Status

ScanNum will be incremented whenever a scan request is received. It
will be set to zero when a "reset scannumber" command is
received.

ScanStatus will be set to 202 whenever a request is received that
is to be forwarded to Spec D. It will be updated with the
returned status whenever that arrives. If the client or algorithm
wants to guarantee a change in status values, it will have to use
the "reset status" command and wait to see the zero status. It is
illegal to issue a "reset status" while a request is pending,
since this could result in loss of status. This condition can be
detected when the "reset status" is received, so an error status
can be returned to the client and logged with memo.

It should be noted that according to this design, the ScanNum
will be incremented for all scan commands.
Scans that fail and are reissued will also get new scan
numbers. This is in conflict with some assumptions made by the
kinetics folks regarding sequential scan numbering. The command
to set the scan number to an explicit value can be used to
guarantee that scans are numbered with the expected values.

Commandline:

  -a n     Sets da_cache address for ScanNum. ScanStatus' address
           will be n+1
  -H host  Specifies the hostname of the SpecD

*/
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
#include <sys/kernel.h>
#include <sys/name.h>
#include <netinet/in.h>
#include <unix.h>
#include <ctype.h>

#include "oui.h"
#include "nortlib.h"
#include "da_cache.h"
#include "specq.h"

static int Host_socket;
static char *HostName = "mattson";
static unsigned short cache_addr;
static int specq_host_init( char *RemHost );
static void specq_parent( pid_t child_pid );
static void update_status( unsigned short status );
static void update_scannum( unsigned short scannum );
static void specq_child(void);
static int child_reply( pid_t child_pid, void *msg, int msgsize );
static int Reply_Quit( pid_t child_pid );

int main(int argc, char **argv ) {
  pid_t child_pid;
  
  child_pid = fork();
  oui_init_options(argc, argv);
  if (child_pid) specq_parent(child_pid);
  else specq_child();
  nl_error( 0, "Terminated" );
  return 0;
}

static void specq_parent( pid_t child_pid ) {
  /* The parent will run in a continuous Receive() loop servicing
     requests from client(s) and from the child. It will enqueue
     requests from the client(s) and Reply() them to the child.
  */
  pid_t who, quit_proxy = 0;
  specq_msg_t msg;
  specq_reply_t rep;
  specq_msg_t fwd;
  int busy = 1;
  unsigned short scannum = 0;
  int name_id;

  nl_error( -2, "Parent: Child is %d", child_pid );
  name_id = qnx_name_attach( 0, nl_make_name( "specq", 0 ) );
  if ( name_id == -1 )
        nl_error( 3, "Unable to attach name" );
  update_scannum( scannum );
  update_status( 0 );
  nl_error( 0, "Parent Started (%d)", getpid() );
  
  /* Ask for quit request proxy */
  
  rep.hdr = fwd.hdr = SPECQ_HDR;
  for (;;) {
    who = Receive( 0, &msg, sizeof(msg));
    nl_error( -2, "Received message from %d", who );
    msg.cmdtext[SPECQ_MAX_MSGSIZE-1] = '\0';
    if ( who == -1 ) {
      if ( errno != EINTR )
        nl_error( 2, "Received errno %d on Receive", errno );
    } else {
      if ( who != quit_proxy && msg.hdr != SPECQ_HDR ) {
        if ( who == child_pid ) {
          nl_error( 4,
            "Received invalid command %04X from child(%d)",
            msg.hdr, child_pid );
        } else {
          nl_error( 2, "Invalid header received from %u", who );
          rep.rv = 400;
        }
      } else {
        switch ( busy ) {
          case 0:
            if ( who == child_pid )
              nl_error( 4, "Received from child while not busy" );
            else if ( who == quit_proxy || msg.cmd == SPECQ_EXIT ) {
              Reply_Quit(child_pid);
              busy = 3;
              update_status( 202 );
              rep.rv = 202;
            } else {
              switch ( msg.cmd ) {
                case SPECQ_RESET:
                case SPECQ_CHECK:
                  if ( child_reply( child_pid, &msg, sizeof(rep))) {
                    rep.rv = 504;
                  } else {
                    rep.rv = 202;
                    busy = 1;
                  }
                  update_status( rep.rv );
                  break;
                case SPECQ_SCAN:
                  fwd.cmd = SPECQ_SCAN;
                  snprintf( fwd.cmdtext, SPECQ_MAX_MSGSIZE-1, msg.cmdtext, ++scannum );
                  fwd.cmdtext[SPECQ_MAX_MSGSIZE] = '\0';
                  if ( child_reply( child_pid, &fwd, sizeof(fwd) ) ) {
                    rep.rv = 504;
                  } else {
                    rep.rv = 202;
                    busy = 1;
                    update_scannum( scannum );
                    update_status( 202 );
                  }
                  break;
                case SPECQ_RESET_SCAN:
                  scannum = atoi(msg.cmdtext);
                  rep.rv = 200;
                  /* update_scannum( scannum ); */
                  update_status( rep.rv );
                  break;
                case SPECQ_RESET_STATUS:
                  rep.rv = 200;
                  update_status( 0 );
                  break;
                default:
                  rep.rv = 400;
                  nl_error( 2, "Invalid request command: %04X", msg.cmd );
                  break;
              }
            }
            break;
          case 1:
            if ( who == child_pid ) {
              update_status( msg.cmd );
              if ( msg.cmd == 410 ) {
                nl_error( 2, "Child is terminating unexpectedly" );
                rep.rv = 0;
                child_reply( child_pid, &rep, sizeof(rep) );
                return;
              }
              busy = 0;
              rep.rv = 0;
            } else if ( who == quit_proxy || msg.cmd == SPECQ_EXIT ) {
              rep.rv = 202;
              busy = 2;
            } else {
              rep.rv = 503;
            }
            break;
          case 2: /* A quit request is pending */
            rep.rv = 0;
            if ( who == child_pid ) {
              update_status( msg.cmd );
              if ( msg.cmd == 410 ) {
                nl_error( 2, "Child is terminating unexpectedly" );
                rep.rv = 0;
                child_reply( child_pid, &rep, sizeof(rep) );
                return;
              }
              Reply_Quit(child_pid);
              busy = 3;
            } else rep.rv = 503;
            break;
          case 3: /* We've already told child to quit */
            if ( who == child_pid ) {
              update_status(msg.cmd);
              if ( msg.cmd == 410 ) {
                rep.rv = 0;
                child_reply( child_pid, &rep, sizeof(rep) );
                return;
              } else nl_error( 4, "Unexpected child non-termination" );
            }
            rep.rv = 503;
            break;
          default:
            nl_error( 4, "Invalid busy state %d", busy );
        }
      }
      if ( who != quit_proxy && rep.rv )
        Reply( who, &rep, sizeof(rep) );
    }
  }
}

static int child_reply( pid_t child_pid, void *msg, int msgsize ) {
  if ( Reply( child_pid, msg, msgsize )) {
    nl_error( 2, "Error %d on Reply to child: %s",
      errno, strerror(errno) );
    return 1;
  }
  return 0;
}

static int Reply_Quit( pid_t child_pid ) {
  specq_reply_t rep;
  rep.hdr = SPECQ_HDR;
  rep.rv = SPECQ_EXIT;
  return child_reply( child_pid, &rep, sizeof(rep) );
}


static void update_status( unsigned short status ) {
  cache_write( cache_addr+1, status );
}

static void update_scannum( unsigned short scannum ) {
  cache_write( cache_addr, scannum );
}

static void specq_child( void ) {
  pid_t parent;
  specq_reply_t rep;
  specq_msg_t msg;
  int ok;

  parent = getppid();
  rep.hdr = SPECQ_HDR;
  nl_error( 0, "Child Started (%d)", getpid() );
  nl_error( -2, "Child: HostName = '%s'", HostName );
  if ( specq_host_init( HostName ) == 0 ) {
    nl_error( -2, "Child: Host init succeeded" );
    /* Initialize connection to spectrometer */
    rep.rv = Send_to_spec( "%s\n", "open specd!" );
    nl_error( -2, "Child: open specd returned %d", rep.rv );
    
    if ( rep.rv/100 == 2 ) {
      rep.rv = Send_to_spec( "%s\n", "check" );
    } else {
      nl_error( 2, "Initial negotiation failed" );
    }
    ok = rep.rv/100 == 2;
    while (ok) {
      nl_error( -2, "Sending status %04X:%04X to parent(%d)",
        rep.hdr, rep.rv, parent );
      if ( Send( parent, &rep, &msg, sizeof(rep), sizeof(msg) ) == 0 ) {
        switch ( msg.cmd ) {
          case SPECQ_EXIT:
            ok = 0;
            break;
          case SPECQ_RESET:
            rep.rv = Send_to_spec( "%s\n", "reset" );
            if ( rep.rv/100 != 2 )
              nl_error( 2, "Reset returned %d", rep.rv );
            break;
          case SPECQ_CHECK:
            rep.rv = Send_to_spec( "%s\n", "check" );
            if ( rep.rv/100 != 2 )
              nl_error( 2, "Check returned %d", rep.rv );
            break;
          case SPECQ_SCAN :
            rep.rv = Send_to_spec( "scan %s\n", msg.cmdtext );
            if ( rep.rv/100 != 2 )
              nl_error( 2, "Scan returned %d", rep.rv );
            break;
          case SPECQ_RESET_SCAN:
          case SPECQ_RESET_STATUS:
          default:
            rep.rv = 400; /* Bad Request */
            break;
        }
      } else if ( errno == ESRCH ) {
        parent = 0;
        nl_error( 2, "Parent terminated" );
        ok = 0;
      } else if ( errno != EINTR ) {
        nl_error( 2, "Error %d sending to parent: %s",
          errno, strerror(errno) );
      }
    }
    /* Shut down connection (if any) */
    rep.rv = Send_to_spec( "%s\n", "exit" );
    if ( rep.rv != 410 )
      nl_error( 2, "Spectrometer returned %d instead of 410", rep.rv );
    close( Host_socket );
  }
  rep.rv = 410;
  /* Notify parent (if any) that shutdown is complete */
  Send( parent, &rep, &msg, sizeof(rep), sizeof(msg) );
  return;
}

/* This version reads a line, expecting a numeric status
   code as the leading characters. Returns 0 on error.
*/
static int tmreadline( int socket ) {
  char buf[MAX_PROTOCOL_LINE], *bfr;
  size_t nb, tb = 0, nbytes;

  nbytes = MAX_PROTOCOL_LINE-1;
  bfr = buf;
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
  }
  return 0;
}

static int Send_to_spec( char *fmt, char *txt ) {
  int rv;
  char buf[MAX_PROTOCOL_LINE+1];

  if ( snprintf( buf, MAX_PROTOCOL_LINE+1, fmt, txt ) ==
                MAX_PROTOCOL_LINE ) {
    nl_error( 2, "Requested command too long for transmission" );
        return 400;
  }
  rv = write( Host_socket, buf, strlen(buf) );
  if ( rv == -1 ) {
        child_error( "writing to socket" );
        return 404; /* Don't know what to do! */
  }
  rv = tmreadline( Host_socket );
  return rv;
}

/* Prints an error message and returns 1 */
static int child_error( char *txt ) {
  nl_error( 2, "%s(%d) %s", strerror(errno), errno, txt );
  return 1;
}

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
  if ( Host_socket == -1 ) return child_error( "from socket()" );
  server.sin_len = 0;
  server.sin_family = AF_INET;
  server.sin_port = Port;
  memcpy( &server.sin_addr, host->h_addr, host->h_length );
  status = connect( Host_socket, (struct sockaddr *)&server, sizeof(server) );
  if ( status == -1 ) return child_error( "from connect()" );
  return 0;
}

void specq_opt_init( int argc, char **argv ) {
  int c;
  unsigned long l_cache_addr;

  optind = 0; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt( argc, argv, opt_string )) != -1) {
    switch ( c ) {
      case 'H':
        HostName = optarg;
        break;
      case 'a':
        l_cache_addr = strtoul( optarg, NULL, 0 );
        cache_addr = (unsigned short) l_cache_addr;
        break;
      case '?':
        nl_error(3, "Unrecognized Option -%c", optopt);
      default:
        /* Another init routine can handle other options */
        break;
    }
  }
}
