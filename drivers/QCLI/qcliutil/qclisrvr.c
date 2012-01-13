/* qclisrvr.c
  Manages access to the QCLI through the Analog I/O board.
*/
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/select.h>
#include "nortlib.h"
#include "nl_assert.h"
#include "qcliutil.h"
#include "oui.h"
#include "qclid.h"
#include "collect.h"
#include "tm.h"

/* Use msg_hdr for the name. */
static const char *qcli_name = "QCLI";

enum qcli_cmd { SW, RW, ST, CE, TN, TF, TP, D0, D1, D2, D3, QU, XX };

typedef struct {
  enum qcli_cmd index;
  char *cmd;
  int takes_arg;
  int value;
} qcli_cmd_def, *qcli_cmd_defp;

qcli_cmd_def qcli_cmds[] = {
  { SW, "SW", 1, 0 },
  { RW, "RW", 0, 0 },
  { ST, "ST", 0, 0 },
  { CE, "CE", 0, 0 },
  { TN, "TN", 1, 0 },
  { TF, "TF", 1, 0 },
  { TP, "TP", 1, 0 },
  { D0, "D0", 1, 0 },
  { D1, "D1", 1, 0 },
  { D2, "D2", 1, 0 },
  { D3, "D3", 1, 0 },
  { QU, "QU", 0, 0 },
  { XX, NULL, 0, 0 }
};

/**
 * Handling the EOF case (read returns 0) will be external to parse_cmd(). Here we are given
 * a NUL-terminated string, and we'll just complain if it is malformed. 
 */

qcli_cmd_defp parse_cmd ( char *cmd ) {
  char *s = cmd;
  int i;
  qcli_cmd_def *cd;
  
  while ( isspace(*s) ) ++s;
  for ( i = 0; qcli_cmds[i].cmd; i++ ) {
    char *mcmd = qcli_cmds[i].cmd;
    if ( s[0] == mcmd[0] && s[1] == mcmd[1] ) break;
  }
  cd = &qcli_cmds[i];
  if ( cd->index == XX ) {
    nl_error( 2, "Unrecognized command: '%s'", cmd );
  } else if ( cd->takes_arg ) {
    if ( s[2] != ':' ) {
      nl_error( 2, "Command '%s' requires argument. Received '%s'", cd->cmd, cmd );
    } else {
      char *arg;
      s += 3;
      arg = s;
      if ( *s == '-' ) ++s;
      if ( ! isdigit(*s) ) {
        nl_error( 2, "Command '%s' requires numeric argument. Received '%s'",
                cd->cmd, cmd );
      } else {
        cd->value = atoi(arg);
        return cd;
      }
    }
  } else if ( s[2] == ':' ) {
    nl_error( 2, "Command '%s' does not take an argument. Received '%s'",
              cd->cmd, cmd );
  } else if ( s[2] != '\0' && !isspace(s[2]) ) {
    nl_error( 2, "Unrecognized command: '%s'", cmd );
  } else return cd;
  return NULL;
}

void qclid_init_options( const char *msg_hdr, int argc, char **argv ) {
  qcli_name = msg_hdr;
}

qcli_data_t qcli_data;

#define CMDEE_BUFSIZE 160
// Return non-zero on EOF
int read_cmd( int cmd_fd ) {
  qcli_cmd_defp cd;
  char buf[CMDEE_BUFSIZE];
  int nb = read( cmd_fd, buf, CMDEE_BUFSIZE-1 );
  if ( nb == -1 )
    nl_error(3, "Error reading from cmd/%s: %s", qcli_name, strerror(errno) );
  if ( nb == 0 ) return 1;
  nl_assert(nb < CMDEE_BUFSIZE);
  buf[nb] = '\0';
  nl_error( -2, "%s cmd received '%s'", qcli_name, buf );
  cd = parse_cmd(buf);
  if ( cd != NULL ) {
    switch ( cd->index ) {
      case SW:
        wr_stop_qcli( QCLI_SELECT_WAVEFORM + cd->value + 8 );
        qcli_data.qcli_wave = cd->value;
        break;
      case RW:
        wr_stop_qcli( QCLI_RUN_WAVEFORM );
        break;
      case ST:
        wr_stop_qcli( QCLI_STOP );
        break;
      case CE:
        wr_stop_qcli( QCLI_CLEAR_ERROR );
        break;
      case TN:
        write_qcli( QCLI_LOAD_MSB + ((cd->value >> 8) & 0xFF) );
        wr_stop_qcli( QCLI_WRITE_TON + (cd->value & 0xFF) );
        break;
      case TF:
        write_qcli( QCLI_LOAD_MSB + ((cd->value >> 8) & 0xFF) );
        wr_stop_qcli( QCLI_WRITE_TOFF + (cd->value & 0xFF) );
        break;
      case TP:
        write_qcli( QCLI_LOAD_MSB + ((cd->value >> 8) & 0xFF) );
        wr_stop_qcli( QCLI_WRITE_TPRE + (cd->value & 0xFF) );
        break;
      case D0:
        write_qcli( QCLI_SELECT_DAC + 0 );
        write_qcli( QCLI_LOAD_MSB + ((cd->value >> 8) & 0xFF) );
        wr_stop_qcli( QCLI_WRITE_DAC + (cd->value & 0xFF) );
        break;
      case D1:
        write_qcli( QCLI_SELECT_DAC + 1 );
        write_qcli( QCLI_LOAD_MSB + ((cd->value >> 8) & 0xFF) );
        wr_stop_qcli( QCLI_WRITE_DAC + (cd->value & 0xFF) );
        break;
      case D2:
        write_qcli( QCLI_SELECT_DAC + 2 );
        write_qcli( QCLI_LOAD_MSB + ((cd->value >> 8) & 0xFF) );
        wr_stop_qcli( QCLI_WRITE_DAC + (cd->value & 0xFF) );
        break;
      case D3:
        write_qcli( QCLI_SELECT_DAC + 3 );
        write_qcli( QCLI_LOAD_MSB + ((cd->value >> 8) & 0xFF) );
        wr_stop_qcli( QCLI_WRITE_DAC + (cd->value & 0xFF) );
        break;
      case QU:
        return 1;
      default: nl_error( 4, "Unknown command" );
    }
  }
  return 0;
}

#ifdef DEBUG
static void mark_status( char c ) {
  fputchar(c);
  fflush(stdout);
}
#else
  #define mark_status(x)
#endif

int main( int argc, char **argv ) {
  int done = 0;
  int sel_width;
  int cmd_fd;
  send_id tm_data;
  fd_set readfds, writefds;
  
  oui_init_options(argc, argv);
  nl_error( 0, "Startup" );
  if ( qcli_diags( 0 )) /* for now, ignore the result, just log */
    nl_error(0,"QCLI Passed Diagnostics");

  // open connections to cmd server and collection
  cmd_fd = ci_cmdee_init( qcli_name );
  tm_data = Col_send_init(qcli_name, &qcli_data, sizeof(qcli_data), 0);
  sel_width = ( cmd_fd > tm_data->fd ? cmd_fd : tm_data->fd ) + 1;
  
  while ( !done ) {
    int n_ready;
    FD_ZERO( &readfds );
    FD_ZERO( &writefds );
    FD_SET( cmd_fd, &readfds );
    FD_SET( tm_data->fd, &writefds );

    mark_status('S');
    n_ready = select( sel_width, &readfds, &writefds, NULL, NULL );
    mark_status('s');
    if ( n_ready == -1 ) nl_error( 3, "Error from select: %s", strerror(errno));
    if ( n_ready == 0 ) nl_error( 3, "select() returned zero" );
    if ( FD_ISSET(cmd_fd, &readfds) ) {
      mark_status('R');
      if ( read_cmd( cmd_fd ) )
	done = 1;
      mark_status('r');
    }
    if ( FD_ISSET(tm_data->fd, &writefds ) ) {
      qcli_data.status = read_qcli(0);
      mark_status('T');
      Col_send(tm_data);
      mark_status('t');
    }
  }
  nl_error( 0, "Shutdown" );
  return 0;
}
