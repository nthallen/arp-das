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
   tcp_socket: ssp TCP: alternately reading and writing: IDLE,WRITE,READ,CONNECT
   udp_socket: ssp UDP: open and closed, always reading: IDLE,READ
 */
#include <fcntl.h> // For cmdee_init
#include <sys/select.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "oui.h"
#include "nortlib.h"
#include "nl_assert.h"
#include "tm.h"
#include "collect.h"
#include "sspint.h"

static char board_hostname[40] = "10.0.0.200";
static char *mlf_config = NULL;
static const char *ssp_name, *interface, *portspec = "0";
#define SSP_AMP_NAME_SIZE 20
static char ssp_amp_name[SSP_AMP_NAME_SIZE];
static int quit_received = 0;
static int trigger_count = 0;
static int latency = 1;
ssp_config_t ssp_config;
noise_config_t noise_config;
ssp_data_t ssp_data;
ssp_amp_data_t ssp_amp_data;
static volatile int saw_sigpipe = 0;

void sspdrv_init( const char *name, int argc, char * const *argv ) {
  int c, board_id;

  ssp_name = name;
  optind = OPTIND_RESET; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'b':
        board_id = atoi(optarg);
        if ( board_id < 0 || board_id > 54 )
          nl_error( 3, "Invalid board_id: %d", board_id );
        snprintf( board_hostname, 40, "10.0.0.%d", 200 + board_id );
        break;
      case 'H':
        snprintf( board_hostname, 40, "%s", optarg );
        break;
      case 'I':
        interface = optarg;
        break;
      case 'N':
        mlf_config = optarg;
        break;
      case 'L':
        latency = atoi(optarg);
        break;
      case '?':
        nl_error(3, "Unrecognized Option -%c", optopt);
    }
  }
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

static char *read_num( char *head, int *newval ) {
  char *tail = head;
  if ( !is_eocmd(*++tail) && *++tail == ':' ) {
    char *num = ++tail;
    if ( *tail == '-' ) ++tail;
    if ( isdigit(*tail) ) {
      while ( isdigit(*tail) ) ++tail;
      if ( is_eocmd(*tail) ) {
        *newval = atoi(num);
        return tail;
      }
    }
  }
  report_invalid(head);
  return NULL;
}

/** Read a command line from cmd_fd (cmd/SSPn)
 I will accept space-delimited combinations of the following:
 Commands:
   Allowed when not acquiring:
    -EN Enable
    NS:xxxx N_Samples
    NA:xxxx N_Average (Pre-Adder)
    NC:xxxx N_Coadd
    NF:xx      Frequency (translate to a divisor)
    NE:x 1-7 bit-mapped Specifies which channels are enabled

   Allowed anytime:
    -DA Disable
    TU:[-]xxxxx Level Trigger Rising
    TD:[-]xxxxx Level Trigger Falling
    TS:x (0-3) Specifies the trigger source
    AE Autotrig Enable
    AD Autotrig Disable
    LE Logging Enable
    LD Logging Disable
    nZ Number of samples with laser off at the start of scan
    nN Starting sample for noise calculations
    nM Ending sample for noise calculations
    XR Reset TCP and UDP connections
    XX Terminate the driver

   Never allowed:
    EX Quit: don't do it!
    NP:xxxxx UDP Port Number (I specify this)
 
  As before, we will accept triggering commands anytime and other command only when not acquiring.
 */
#define CMDEE_BUFSIZE 160
void read_cmd( int cmd_fd ) {
  char buf[CMDEE_BUFSIZE], *head, *tail;
  int nb, newval;
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
  nl_error( -2, "sspdrv cmd received '%s'", buf );
  head = buf;
  while ( *head ) {
    while ( isspace(*head) ) ++head;
    tail = head;
    /* In the following switch statement, if we break out, the command code
       will be directly transmitted to the SSP. If instead we continue, the
       code will not be transmitted.  Hence any codes that are handled
       entirely in the driver must set head=tail and use continue.
       This applies to LD, LE, XR, XX, nZ, nN and nM as well as any command
       that is incorrectly formatted or inappropriate due to the current
       operating mode.
     */
    switch (*head) {
      case '\0': continue;
      case 'E':
        if ( *++tail != 'N' || !is_eocmd(*++tail) ) {
          report_invalid(head);
          return;
        }
        if ( udp_state != FD_IDLE ) {
          nl_error( 2, "EN not valid: Already enabled" );
          return;
        }
        if ( tcp_state == FD_CONNECT ) {
          nl_error( 2, "EN suppressed: not connected" );
          head = tail;
          continue;
        }
        { ssp_config.NP = udp_create(interface, portspec);
          char udp_buf[20];
          snprintf(udp_buf, 20, "NP:%d", ssp_config.NP);
          tcp_enqueue(udp_buf);
        }
        ssp_data.Status = SSP_STATUS_ARMED;
        break;
      case 'D':
        if ( *++tail != 'A' || !is_eocmd(*++tail) ) {
          report_invalid(head);
          return;
        }
        udp_close();
        if ( tcp_state != FD_CONNECT )
          ssp_data.Status = SSP_STATUS_READY;
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
      case 'L':
        switch (*++tail) {
          case 'E':
            if ( is_eocmd(*++tail) ) {
              ssp_config.LE = 1;
              head = tail;
              continue;
            } else {
              report_invalid(head);
              return;
            }
          case 'D':
            if ( is_eocmd(*++tail) ) {
              ssp_config.LE = 0;
              head = tail;
              continue;
            } else {
              report_invalid(head);
              return;
            }
          default:
            report_invalid(head);
            return;
        }
        break;
      case 'X':
        switch (*++tail) {
          case 'R':
            udp_close();
            tcp_reset(board_hostname);
            head = ++tail;
            continue;
          case 'X':
            udp_close();
            head = ++tail;
            quit_received = 1;
            continue;
          default:
            report_invalid(head);
            return;
        }
        break;
      case 'N':
        tail = read_num( head, &newval );
        if ( tail == NULL ) return;
        if ( udp_state != FD_IDLE ) {
          *tail = '\0';
          nl_error( 2, "Command invalid while SSP is enabled: '%s'", head );
          return;
        }
        switch (head[1]) {
          case 'S': ssp_config.NS = newval; noise_config.modified = 1; break;
          case 'A': ssp_config.NA = newval; break;
          case 'C': ssp_config.NC = newval; break;
          case 'F':
            { char div_buf[80];
              ssp_config.NF = 100000000/newval;
              snprintf(div_buf, 80, "NF:%d", ssp_config.NF );
              tcp_enqueue(div_buf);
            }
            head = tail;
            continue;
          case 'E': ssp_config.NE = newval; break;
          default:
            report_invalid(head);
            return;
        }
        break;
      case 'n':
        tail = read_num( head, &newval );
        if ( tail == NULL ) return;
        switch (head[1]) {
          case 'Z':
            noise_config.NZ = newval;
            noise_config.modified = 1;
            head = tail;
            continue;
          case 'N':
            noise_config.NN = newval;
            noise_config.modified = 1;
            head = tail;
            continue;
          case 'M':
            noise_config.NM = newval;
            noise_config.modified = 1;
            head = tail;
            continue;
          default:
            report_invalid(head);
            return;
        }
        break;
      case 'T': // Trigger commands
        tail = read_num( head, &newval );
        if ( tail == NULL ) return;
        switch (head[1]) {
          case 'U':
          case 'D':
          case 'S':
            break;
          default:
            report_invalid(head);
            return;
        }
        break;
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
  if (noise_config.modified) {
    if (noise_config.NN == 0 || noise_config.NM == 0) {
      noise_config.NZ = 0;
    }
    if (noise_config.NN > noise_config.NM ||
        noise_config.NZ >= noise_config.NN ||
        noise_config.NM > ssp_config.NS) {
      noise_config.NZ = 0;
    }
    if (noise_config.NZ == 0) {
      int i;
      noise_config.NN = 0;
      noise_config.NM = 0;
      for (i = 0; i < 3; ++i) {
        ssp_amp_data.noise[i] = 0;
        ssp_amp_data.noise_percent[i] = 0;
      }
    } else {
      noise_config.NSamp = noise_config.NM - noise_config.NN + 1;
      noise_config.meanX = (noise_config.NSamp+1)/2.0;
      noise_config.sumX2  = powf(noise_config.NSamp,3)/12. -
        noise_config.NSamp/12.;
    }
    noise_config.modified = 0;
  }
}

void sigpipehandler(int sig) {
  ++saw_sigpipe;
}

int main( int argc, char **argv ) {
  mlf_def_t *mlf;
  int cmd_fd;
  int non_udp_width, udp_width;
  int do_amp = 0;
  send_id tm_data, tm_amp_data;
  fd_set readfds, writefds;
  
  oui_init_options(argc, argv);
  ssp_config.LE = 1; // Logging enabled by default
  nl_error( 0, "Startup: sspdrv V1.5 7/3/18" );
  udp_close(); // Initialize ssp_config and udp_state
  mlf = mlf_init( 3, 60, 1, ssp_name, "dat", mlf_config );
  ssp_data.index = mlf->index;
  ssp_data.ScanNum = 0;
  ssp_data.Flags = 0;
  ssp_data.Total_Skip = 0;
  cmd_fd = ci_cmdee_init( ssp_name );
  tm_data = Col_send_init(ssp_name, &ssp_data, sizeof(ssp_data), 0);
  if (snprintf(ssp_amp_name, SSP_AMP_NAME_SIZE, "%s_amp", ssp_name) >= SSP_AMP_NAME_SIZE) {
    nl_error(MSG_ERROR, "ssp_amp_name exceeds size limit");
  } else {
    int old_response = set_response(NLRSP_QUIET);
    tm_amp_data = Col_send_init(ssp_amp_name, &ssp_amp_data, sizeof(ssp_amp_data), 0);
    set_response(old_response);
    do_amp = tm_amp_data != 0;
  }
  tcp_create(board_hostname);
  signal(SIGPIPE, &sigpipehandler);
  non_udp_width = cmd_fd + 1;
  if ( tm_data->fd >= non_udp_width )
    non_udp_width = tm_data->fd + 1;
  while (!quit_received) {
    int n_ready;
    FD_ZERO( &readfds );
    FD_ZERO( &writefds );
    FD_SET( cmd_fd, &readfds );
    FD_SET( tm_data->fd, &writefds );
    switch ( tcp_state ) {
      case FD_IDLE:
        break;
      case FD_CONNECT:
      case FD_WRITE:
        FD_SET(tcp_socket, &writefds );
        break;
      case FD_READ:
        FD_SET(tcp_socket, &readfds );
        break;
      default: nl_error(4, "Bad case for tcp_state" );
    }
    switch ( udp_state ) {
      case FD_IDLE:
        udp_width = non_udp_width;
        break;
      case FD_READ:
        nl_assert(udp_socket >= 0 && udp_socket < 32);
        FD_SET(udp_socket, &readfds );
        udp_width = udp_socket >= non_udp_width ?
           udp_socket + 1 : non_udp_width;
        break;
      default: nl_error(4, "Bad case for udp_state" );
    }
    if ( tcp_socket >= udp_width )
      udp_width = tcp_socket+1;
    n_ready = select( udp_width, &readfds, &writefds, NULL, NULL );
    if ( n_ready == -1 ) {
      if (errno == EINTR && saw_sigpipe) {
        saw_sigpipe = 0;
        tcp_reset(board_hostname);
        nl_error(2, "Received SIGPIPE, resetting TCP");
      } else {
        nl_error( 3, "Error from select: %s", strerror(errno));
      }
    } else {
      if ( n_ready == 0 ) nl_error( 3, "select() returned zero" );
      if ( udp_state == FD_READ && FD_ISSET( udp_socket, &readfds ) ) {
        udp_read(mlf, do_amp);
        ssp_data.Status = SSP_STATUS_TRIG;
        trigger_count = 0;
      }
      if ( FD_ISSET(cmd_fd, &readfds) )
        read_cmd( cmd_fd );
      if ( FD_ISSET(tm_data->fd, &writefds ) ) {
        if ( ssp_data.Status == SSP_STATUS_TRIG &&
             ++trigger_count > latency+1 )
          ssp_data.Status = SSP_STATUS_ARMED;
        Col_send(tm_data);
        ssp_data.Flags &= ~SSP_OVF_MASK;
        if (tm_amp_data) Col_send(tm_amp_data);
      }
      if ( FD_ISSET(tcp_socket, &readfds ) )
        tcp_recv();
      if ( FD_ISSET(tcp_socket, &writefds ) ) {
        if ( tcp_state == FD_CONNECT ) {
          tcp_connected();
        } else tcp_send();
      }
    }
  }
  nl_error( 0, "Shutdown" );
  ssp_data.Status = SSP_STATUS_GONE;
  Col_send(tm_data);
  return 0;
}
