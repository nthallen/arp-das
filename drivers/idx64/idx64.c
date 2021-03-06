#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include "nortlib.h"
#include "oui.h"
#include "collect.h"
#include "subbus.h"
#include "idx64.h"
#include "idx64int.h"
#include "tm.h"

#ifdef DOCUMENTATION
  Overview of the structure of this application:
  
                         main
                           |
                           v
                        operate
                           |
         +-----------------+----------------+
         |                 |                |
         v                 |                |
    drive_command          |                |   
         |                 |                |
         v                 v                v
    queue_request--> service_board      scan_proxy
                           |                |
                           v                v
                      execute_cmd <--- service_scan
                           |                |
                           |                |
                           +-> drive_chan <-+

  There are several other support routines and initializations,
  but these are the main operating procedures.
  
  operate() preforms the main Receive() loop servicing both 
  commands and interrupts, which come in as proxies. Also when 
  scanning, the scan steps are timed by proxies from TM 
  collection.
  
  Commands are passed to to drive_command() which immediately
  executes those that do not touch the hardware and passes the 
  rest on to queue_request().
  
  queue_request() does just that, sets a flag requesting service 
  and then calls service_board() to determine if the channel is 
  ready to service the request.
  
  service_board() is the interrupt service routine. It is passed 
  the index of the board which produced the interrupt. It reads 
  the board''s status register and compares that to the request 
  word to determine if any channels need attention. If so, the 
  channel is passed on to execute_cmd().
  
  execute_cmd() is what actually processes the commands which 
  address the hardware. It translates online/offline/altline
  commands to absolute positions and then translates absolute
  positions to relative drives. If the command is a scan, 
  appropriate flags are set and no further action is taken. For 
  drives, the hardware is programmed accordingly and the command 
  is dequeued. If no more commands are enqueued, the request flag 
  is cleared indicating that this channel does not require 
  service when the drive is completed.
  
  scan_proxy() services the TM proxy which drives the scan rate.
  If surveys all of the boards for scans in progress and calls 
  service_scan() for each scan ready for action.
  
  service_scan() issues each drive command directly. When the 
  scan is completed, it dequeues the scan command and calls 
  execute_cmd() to handle the next command in the queue.
#endif

/* Global Variables */
idx64_def idx_defs[ MAX_IDXRS ] = {
  { 0xA00, "Idx64_0" },
  { 0xA40, "Idx64_1" },
  { 0xA80, "Idx64_2" },
  { 0xAC0, "Idx64_3" }
};
idx64_bd *boards[ MAX_IDXRS ];
/* If every channel requires 3 status bits (maximum possible), we
   can fit 16/3 = 5 channel's status in a 16-bit word. Max
   number of channels is MAX_IDXRS*MAX_IDXR_CHANS. Add 4 and
   divide by 5 to get the max number of 16-bit words required
   to hold all the status bits.
*/
#define MAX_WORDS ((MAX_IDXRS*MAX_IDXR_CHANS+4)/5)
unsigned short tm_ptrs[ MAX_WORDS ];
send_id tm_data;
static int cmd_fd = -1;
static struct sigevent cmd_event;
char *idx64_cfg_string;
int idx64_region = 0x40;
static int n_scans = 0;

#define CMD_PULSE_CODE (_PULSE_CODE_MINAVAIL + 0)
#define SCAN_PULSE_CODE (_PULSE_CODE_MINAVAIL + 1)
#define EXPINT_PULSE_CODE (_PULSE_CODE_MINAVAIL + 2)

/* Channel_init() initializes the channel's data structure,
   zeros the hardware's counters and performs default
   configuration.

   Channel_init() is distinguished from config_channels()
   which performs additional configuration of the channels
   based on command-line input.
*/
static void Channel_init( chandef *chan, unsigned short base ) {
  chan->base_addr = base;
  chan->tm_ptr = NULL;
  chan->scan_bit = 0;
  chan->on_bit = 0;
  chan->supp_bit = 0;
  chan->online = 0;
  chan->online_delta = 0;
  chan->offline_delta = 0;
  chan->offline_pos = 0;
  chan->altline_delta = 0;
  chan->altline_pos = 0;
  chan->hysteresis = 0;
  chan->first = NULL;
  chan->last = NULL;
  sbwr( base+6, DFLT_CHAN_CFG );
  sbwr( base+4, 0 ); /* set position to 0 */
  sbwr( base+2, 0 ); /* drive out 0 */
}

/* init_boards will check for the presence of each possible 
  indexer board. If present, it will allocate a structure for
  the board and request a proxy for the board from intserv.
  If intserv doesn't allow us, it is probably because the board
  is already owned by someone else, a pretty good fatal error.
*/
static void init_boards( int coid ) {
  int i, j;
  idx64_bd *bd;
  idx64_def *bddef;
  struct sigevent event;

  event.sigev_notify = SIGEV_PULSE;
  event.sigev_coid = coid;
  event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
  event.sigev_code = EXPINT_PULSE_CODE;
  // event.sigev_value.sival_int = 0; //redefined for each board.
  for ( i = 0; i < MAX_IDXRS; i++ ) {
    unsigned short val;

    /* if present, allocate structure, get proxy */
    bddef = &idx_defs[i];
    if ( read_ack( bddef->card_base, &val ) != 0 ) {
      nl_error( -2, "Board %s present", bddef->cardID );
      bd = boards[i] = new_memory( sizeof( idx64_bd ) );
      event.sigev_value.sival_int = bd->pulse_value = i;
      subbus_int_attach( bddef->cardID, bddef->card_base,
      	idx64_region, &event );
      bd->request = bd->scans = 0;
      for ( j = 0; j < MAX_IDXR_CHANS; j++ )
        Channel_init( &bd->chans[j], bddef->card_base + 8 * ( j + 1 ) );
    } else nl_error( -2, "Board %s not present", bddef->cardID );
  }
}

/* idx_cfg_num invokes strtoul() to read a number, verify that a
   number was in fact read (pointer updated) and updates the 
   pointer. idx_cfg_num dies fatally if there was no number
*/
static unsigned short idx_cfg_num( char **s, int base ) {
  char *t;
  unsigned short val;
  
  t = *s;
  val = strtoul( t, s, base );
  if ( t == *s )
    nl_error( 3, "Syntax error in configuration string" );
  return val;
}


static void tm_status_set( unsigned short *ptr,
                unsigned short mask, unsigned short value ) {
  if ( ptr != 0 ) {
    //unsigned short old = *ptr;
    *ptr = ( *ptr & ~mask ) | ( value & mask );
    //nl_error(-2, "tm_status_set: old = %04X new = %04X", old, *ptr );
    //if ( *ptr != old ) {
    //  if ( Col_send( tm_data ) )
    //  nl_error(-3, "Col_send() returned error %d", errno);
    //  else nl_error(-3, "Col_send() succeeded" );
    //}
  } else nl_error( -3, "tm_status_set(NULL)" );
}

/*
    [cfg code][,n_bits][:[cfg code][,n_bits] ...]
    no spaces, default cfg code is C00 (hex). default n_bits is 0
    Later may add ability to read the configuration from a file, 
    but that's a low priority.
*/
static void config_channels( char *s ) {
  int chan = 0;
  unsigned short code, bits;
  int wdno, bitno;
  int bdno, chno;
  idx64_bd *bd;
  chandef *ch;
  
  wdno = bitno = 0;
  while ( *s != '\0' ) {
    if ( *s == ':' ) {
      s++; /* empty def */
      chan++;
    } else {
      /* non-empty definition */
      if ( *s != ',' ) code = idx_cfg_num( &s, 16 );
      else code = DFLT_CHAN_CFG;
      if ( *s == ',' ) {
        s++;
        bits = idx_cfg_num( &s, 10 );
        if ( bits > 3 )
          nl_error( 3, "bits value greater than 3" );
      } else bits = 0;
      if ( bitno + bits > 16 ) {
        wdno++;
        bitno = 0;
      }
      bdno = chan / MAX_IDXR_CHANS;
      chno = chan % MAX_IDXR_CHANS;
      if ( bdno > MAX_IDXRS )
        nl_error( 3, "Too many channel configurations" );
      bd = boards[ bdno ];
      if ( bd == 0 ) {
        nl_error( 1, "Configuration specified for non-existant channel" );
        bitno += bits;
      } else {
        ch = &bd->chans[ chno ];
        if ( bits != 0 ) {
          ch->tm_ptr = &tm_ptrs[ wdno ];
          if ( bits & 1 )
            ch->scan_bit = ( 1 << bitno++ );
          if ( bits >= 2 ) {
            ch->on_bit = ( 1 << bitno++ );
            ch->supp_bit = ( 1 << bitno++ );
          }
        }
        if ( code != DFLT_CHAN_CFG )
          sbwr( ch->base_addr + 6, code );
      }
    }
  }
}

static void dequeue( chandef *ch ) {
  ixcmdl *im;

  if ( ch != 0 && ch->first != 0 ) {
    im = ch->first;
    ch->first = im->next;
    if ( ch->first == 0 ) ch->last = NULL;
    free( im );
  }
}

/* scan_setup() is used to start and stop scans.
   If start != 0, it sets the scans bit in the bd def and keeps track
   of the number of scans currently active. When the number of
   scans goes up from zero to 1, the scan proxy is requested
   from collection. If that fails, scan_setup will not init
   and will return non-zero.

   If start==0, this is a cleanup command. The scans bit is 
   cleared (if set), and if the global number of scans drops to 
   zero, the scan proxy is reset.
*/ 
static int scan_setup( idx64_bd *bd, unsigned short chno, int start ) {

  if ( start != 0 ) {
    if ( ( bd->scans & (1<<chno) ) == 0 ) {
      n_scans++;
      bd->scans |= (1<<chno);
    }
  } else {
    if ( ( bd->scans & (1<<chno) ) != 0 ) {
      assert( n_scans > 0 );
      --n_scans;
      bd->scans &= ~(1<<chno);
    }
  }
  return 0;
}

static unsigned short stop_channel( idx64_bd *bd, unsigned short chno ) {
  chandef *ch;
  
  assert( bd != 0 && chno < MAX_IDXR_CHANS );
  ch = &bd->chans[ chno ];

  /* Issue the stop command to the hardware */
  sbwr( ch->base_addr + 2, 1 ); /* Drive out 1 */

  /* Cancel pending requests */
  while ( ch->first != 0 ) dequeue( ch );
  bd->request &= ~(1 << chno);
  scan_setup( bd, chno, 0 );

  /* Clear all the TM bits */
  tm_status_set( ch->tm_ptr,
      ch->scan_bit | ch->on_bit | ch->supp_bit, 0 );
  return EOK;
}

/* cancel proxy requests from intserv, delete proxies */
static void shutdown_boards( void ) {
  int i, j;
  idx64_bd *bd;

  for ( i = 0; i < MAX_IDXRS; i++ ) {
    bd = boards[i];
    if ( bd != 0 ) {
      /* Stop each channel */
      for ( j = 0; j < MAX_IDXR_CHANS; j++ )
        stop_channel( bd, j );
      subbus_int_detach( idx_defs[i].cardID );
    }
  }
}

step_t translate_steps( chandef *ch, byte_t *cmd, step_t *steps ) {
  step_t curpos, dest;
  
  switch ( *cmd ) {
    case IX64_ONLINE:
      *cmd = IX64_TO;
      *steps = ch->online;
      break;
    case IX64_OFFLINE:
      *cmd = IX64_TO;
      *steps = ch->offline_pos ? ch->offline_pos :
          ch->online + ch->offline_delta;
      break;
    case IX64_ALTLINE:
      *cmd = IX64_TO;
      *steps = ch->altline_pos ? ch->altline_pos :
          ch->online + ch->altline_delta;
      break;
    default:
      break;
  }
  /* Translate IX64_TO to IX64_IN or IX64_OUT */
  curpos = sbrw( ch->base_addr + 4 );
  if (*cmd & IX64_TO) {
    dest = *steps;
    *cmd &= ~IX64_DIR;
    if (curpos < *steps) {
      *cmd |= IX64_OUT;
      *steps -= curpos;
    } else {
      *cmd |= IX64_IN; /* nop! */
      *steps = curpos - *steps;
    }
  } else if ( ( *cmd & IX64_DIR ) == IX64_OUT ) {
    dest = curpos + *steps;
  } else {
    dest = curpos - *steps;
  }
  return dest;
}

/* drive_chan() handles IXCMD_NEEDS_DRIVE and IXCMD_NEEDS_HYST */
static int
drive_chan( chandef *ch, ixcmdl *im ) {
  unsigned short addr, stat;
  byte_t cmd;
  step_t steps;

  if ( im->flags & IXCMD_NEEDS_DRIVE ) {
    im->flags &= ~IXCMD_NEEDS_DRIVE;
    cmd = im->c.dir_scan;
    steps = ( cmd & IX64_SCAN ) ? im->c.dsteps : im->c.steps;
    im->dest = translate_steps( ch, &cmd, &steps );
    if ( (( cmd & IX64_DIR ) == IX64_IN ) &&
        ch->hysteresis != 0 && im->dest - steps < im->dest ) {
      steps += ch->hysteresis;
      im->flags |= IXCMD_NEEDS_HYST;
    }
    switch ( im->c.dir_scan ) {
      case IX64_ONLINE:
      case IX64_OFFLINE:
      case IX64_ALTLINE:
        im->flags |= IXCMD_NEEDS_STATUS;
        break;
      default:
        break;
    }
    tm_status_set( ch->tm_ptr, ch->supp_bit | ch->on_bit, 0 );
  } else if ( im->flags & IXCMD_NEEDS_HYST ) {
    step_t curstep =  sbrw( ch->base_addr + 4 );
    im->flags &= ~IXCMD_NEEDS_HYST;
    if ( im->dest > curstep ) {
      cmd = IX64_OUT;
      steps = im->dest - curstep;
    } else {
      im->flags &= ~IXCMD_NEEDS_STATUS;
      return 0;
    }
  } else {
    nl_error( 4, "drive_chan without drive or hyst" );
  }
  
  /* Check limits and don't drive if we're against one */
  stat = sbrba( ch->base_addr + 6 );
  if ( ( cmd & IX64_DIR) == IX64_OUT ) {
    if ( stat & 2 ) return 0;
    addr = ch->base_addr + 2;
  } else {
    if ( stat & 1 ) return 0;
    addr = ch->base_addr;
  }
  /* drive 0 in opposite dir. to clear limit latch */
  sbwr( addr ^ 2, 0 );
  sbwr( addr, steps );
  return 1;
}

/* execute_cmd() executes the next command on the specified 
   channel's command queue. Except for scans, the command is then 
   dequeued. If no further commands are queued, the corresponding
   requests bit in the board structure is cleared.

   Scan strategy: in addition to requests, keep scans word
   for each board. On interrupt, clear request, on proxy,
   if request bit is clear, issue the command...
*/
static void execute_cmd( idx64_bd *bd, int chno ) {
  chandef *ch;
  ixcmdl *im;

  ch = &bd->chans[ chno ];
  while ( ch->first != 0 ) {
    im = ch->first;
    if ( im->flags & IXCMD_NEEDS_INIT ) {
      nl_error( -2, "Init command on channel %d", chno );
      im->flags &= ~IXCMD_NEEDS_INIT;
      if ( im->c.dir_scan == IX64_PRESET_POS ) {
        sbwr( ch->base_addr + 4, im->c.steps );
        dequeue( ch );
      } else if ( im->c.dir_scan == IX64_SET_SPEED ) {
        sbwr( ch->base_addr + 6, im->c.steps & 0xF00 );
        dequeue( ch );
      } else if ( im->c.dir_scan & IX64_SCAN ) {
        scan_setup( bd, chno, 1 );
        translate_steps( ch, &im->c.dir_scan, &im->c.steps );
        bd->request &= ~(1 << chno);
        return;
      } else {
        /* set up normal drives */
        im->flags |= IXCMD_NEEDS_DRIVE;
      }
    } else if ( im->flags &
                (IXCMD_NEEDS_DRIVE | IXCMD_NEEDS_HYST ) ) {
      nl_error( -2, "Drive command on channel %d", chno );
      if ( drive_chan( ch, im ) ) return;
      if ( (im->c.dir_scan & IX64_SCAN) == 0 )
        dequeue( ch );
    } else if ( im->flags & IXCMD_NEEDS_STATUS ) {
      im->flags &= ~IXCMD_NEEDS_STATUS;
      switch ( im->c.dir_scan ) {
        case IX64_ONLINE:
          tm_status_set( ch->tm_ptr, ch->supp_bit | ch->on_bit,
                                                    ch->on_bit );
          break;
        case IX64_OFFLINE:
          tm_status_set( ch->tm_ptr, ch->supp_bit | ch->on_bit,
                                     ch->supp_bit );
          break;
        case IX64_ALTLINE:
          tm_status_set( ch->tm_ptr, ch->supp_bit | ch->on_bit,
                                     ch->supp_bit | ch->on_bit );
          break;
        default:
          nl_error( 4, "Did not need status!" );
      }
    } else if ( im->c.dir_scan & IX64_SCAN ) {
      bd->request &= ~(1 << chno);
      return;
    } else dequeue(ch);
  }
  if ( ch->first == 0 )
    bd->request &= ~(1 << chno);
}

/* service_board() is called when a board signals an interrupt
   and when a command is enqueued. It will query the board's
   status */
static void service_board( int bdno ) {
  idx64_bd *bd;
  unsigned int chno, mask;

  if ( bdno >= MAX_IDXRS || boards[bdno] == 0 )
    nl_error( 4, "Invalid bdno in service_board" );
  bd = boards[ bdno ];
  mask = bd->request & ~sbrba( idx_defs[ bdno ].card_base );
  /* Mask should now have a non-zero bit for each channel
     which is ready to be serviced. */
  for ( chno = 0; mask != 0 && chno < MAX_IDXR_CHANS; chno++ ) {
    if ( mask & ( 1 << chno ) ) {
      if ( bd->scans & ( 1 << chno ) )
        bd->request &= ~(1 << chno);
      else
        execute_cmd( bd, chno );
      mask &= ~(1 << chno );
    }
  }
}

static unsigned short queue_request( idx64_cmnd *cmd ) {
  ixcmdl *cmdl;
  unsigned int bdno, chno;
  idx64_bd *bd;
  chandef *ch;
  
  bdno = cmd->drive / MAX_IDXR_CHANS;
  chno = cmd->drive % MAX_IDXR_CHANS;
  if ( bdno >= MAX_IDXRS || boards[bdno] == 0 )
    nl_error( 4, "Invalid bdno in queue_request" );
  bd = boards[ bdno ];
  ch = &bd->chans[ chno ];

  cmdl = malloc( sizeof( ixcmdl ) );
  if ( cmdl == 0 ) {
    nl_error( 1, "Out of memory in queue_request!" );
    return ENOMEM;
  }
  cmdl->next = NULL;
  cmdl->flags = IXCMD_NEEDS_INIT;
  cmdl->c = *cmd;
  if ( ch->first == 0 ) ch->first = cmdl;
  else ch->last->next = cmdl;
  ch->last = cmdl;
  bd->request |= (1 << chno);
  service_board( bdno );
  return EOK;
}

/* service_scan() is called from scan_proxy() to service an 
   ongoing scan. It is only called if the scans bit is set and
   the request bit is clear. After each drive, service_scan()
   sets the request bit, and it is cleared by execute_cmd()
   when the drive-completed interrupt is received. As such,
   though we don't actually read the board status here, we
   can be confident that the drive is complete and it is safe
   to drive the next step or to execute another command if
   the scan is complete.
*/
static void service_scan( idx64_bd *bd, int chno ) {
  chandef *ch;
  ixcmdl *im;
  
  ch = &bd->chans[chno];
  im = ch->first;
  assert( im != 0 );
  assert( ch->scan_bit == 0 || ch->tm_ptr != 0 );
  if ( im->c.steps != 0 && ch->scan_bit != 0 &&
        ( (*ch->tm_ptr) & ch->scan_bit ) == 0 ) {
    tm_status_set( ch->tm_ptr, ch->scan_bit, ch->scan_bit );
  } else if ( im->c.steps != 0 ) {
    /* do drive and set request flag */
    if ( im->c.dsteps == 0 ) im->c.dsteps = 1;
    if ( im->c.dsteps > im->c.steps ) im->c.dsteps = im->c.steps;
    im->c.steps -= im->c.dsteps;
    im->flags |= IXCMD_NEEDS_DRIVE;
    if ( drive_chan( ch, im ) )
      bd->request |= ( 1 << chno );
    else im->c.steps = 0;
  } else if ( im->c.dsteps != 0 && ch->scan_bit != 0 ) {
    tm_status_set( ch->tm_ptr, ch->scan_bit, 0 );
    im->c.dsteps = 0;
  } else {
    /* all done: clear scans, dequeue and service next command */
    scan_setup( bd, chno, 0 );
    bd->request |= (1 << chno);
    dequeue( ch );
    execute_cmd( bd, chno );
  }
}

/* scan_proxy() is called when the scan proxy is received */
static void scan_pulse( void ) {
  idx64_bd *bd;
  unsigned short svc;
  int bdno, chno;

  Col_send(tm_data);
  if (n_scans) {
    for ( bdno = 0; bdno < MAX_IDXRS; bdno++ ) {
      bd = boards[bdno];
      if ( bd != 0 && bd->scans != 0 ) {

	/* Clear bd->request for any scans no longer running */
	svc = bd->scans & bd->request &
	      ~sbrba( idx_defs[ bdno ].card_base );
	bd->request &= ~svc;

	/* Now find out which scans are not running */
	svc = bd->scans & ~bd->request;
	nl_error( svc==0 ? -2 : -3, "scan_proxy scans %02X svc %02X",
		    bd->scans, svc );

	for ( chno = 0; svc != 0 && chno < MAX_IDXR_CHANS; chno++ ) {
	  if ( svc & 1 ) service_scan( bd, chno );
	  svc >>= 1;
	}
      }
    }
  }
}

/* drive_command() is called when a drive command message is
   received. It returns the status value which is to be returned
   in the Reply
*/
static unsigned short drive_command( idx64_cmnd *cmd ) {
  unsigned int bdno, chno;
  idx64_bd *bd;
  chandef *ch;

  bdno = cmd->drive / MAX_IDXR_CHANS;
  chno = cmd->drive % MAX_IDXR_CHANS;
  if ( bdno >= MAX_IDXRS || boards[bdno] == 0 ) return ENXIO;
  bd = boards[ bdno ];
  ch = &bd->chans[ chno ];
  
  if ( cmd->dir_scan < IX64_STOP )
    return queue_request( cmd );
  else switch ( cmd->dir_scan ) {
    case IX64_ONLINE:
    case IX64_OFFLINE:
    case IX64_ALTLINE:
    case IX64_PRESET_POS:
    case IX64_SET_SPEED:
      return queue_request( cmd );
    case IX64_STOP:
      return stop_channel( bd, chno );
    case IX64_MOVE_ONLINE_OUT:
      ch->online += ch->online_delta; return EOK;
    case IX64_MOVE_ONLINE_IN:
      ch->online -= ch->online_delta; return EOK;
    case IX64_SET_ONLINE:
      ch->online = cmd->steps; return EOK;
    case IX64_SET_ON_DELTA:
      ch->online_delta = cmd->steps; return EOK;
    case IX64_SET_OFF_DELTA:
      ch->offline_delta = cmd->steps;
      ch->offline_pos = 0;
      return EOK;
    case IX64_SET_OFF_POS:
      ch->offline_pos = cmd->steps; return EOK;
    case IX64_SET_ALT_DELTA:
      ch->altline_delta = cmd->steps;
      ch->altline_pos = 0;
      return EOK;
    case IX64_SET_ALT_POS:
      ch->altline_pos = cmd->steps; return EOK;
    case IX64_SET_HYSTERESIS:
      ch->hysteresis = cmd->steps; return EOK;
    case IX64_QUIT: /* should have been handled in operate() */
    default:
      return ENOSYS; /* Unknown command */
  }
}

#define MAX_IXCMD_ARGS 3
static int parse_command( char *buf, int nb ) {
  int args[MAX_IXCMD_ARGS];
  int i, n_args = 0;
  int args_expected;
  int cmd_code = -1;
  idx64_cmnd cmd;
  int rv;
  
  if ( nb < 3 ) {
    nl_error( 2, "Short command received" );
    return 0;
  }
  nl_error( -2, "Recd: %s", buf );
  for (i = 2; i < nb; ) {
    if (isdigit(buf[i])) {
      int arg = buf[i++] - '0';
      while ( i<nb && isdigit(buf[i])) {
        arg = arg*10 + buf[i++] - '0';
      }
      if (i >= nb) {
        nl_error( 2, "Command truncated inside number" );
        return 0;
      }
      args[n_args++] = arg;
      if ( buf[i] == ':' ) i++;
    } else if (buf[i] == '\n') {
      if (++i < nb ) {
        nl_error( 1, "Garbage after newline in parse_command" );
        break;
      }
    }
  }
  switch (buf[0]) {
    case 'A':
      switch (buf[1]) {
        case 'L': cmd_code = IX64_ALTLINE; args_expected = 1; break;
        case 'P': cmd_code = IX64_SET_ALT_POS; args_expected = 2; break;
        case 'D': cmd_code = IX64_SET_ALT_DELTA; args_expected = 2; break;
        default: break;
      }
      break;
    case 'D':
      switch (buf[1]) {
        case 'I': cmd_code = IX64_IN; args_expected = 2; break;
        case 'O': cmd_code = IX64_OUT; args_expected = 2; break;
        case 'T': cmd_code = IX64_TO; args_expected = 2; break;
        default: break;
      }
      break;
    case 'F':
      switch (buf[1]) {
        case 'D': cmd_code = IX64_SET_OFF_DELTA; args_expected = 2; break;
        case 'P': cmd_code = IX64_SET_OFF_POS; args_expected = 2; break;
        default: break;
      }
      break;
    case 'H':
      if (buf[1] == 'Y') {
        cmd_code = IX64_SET_HYSTERESIS;
        args_expected = 2;
      }
      break;
    case 'M':
      switch (buf[1]) {
        case 'I': cmd_code = IX64_MOVE_ONLINE_IN; args_expected = 1; break;
        case 'O': cmd_code = IX64_MOVE_ONLINE_OUT; args_expected = 1; break;
        default: break;
      }
      break;
    case 'O':
      switch (buf[1]) {
        case 'N': cmd_code = IX64_ONLINE; args_expected = 1; break;
        case 'F': cmd_code = IX64_OFFLINE; args_expected = 1; break;
        case 'P': cmd_code = IX64_SET_ONLINE; args_expected = 2; break;
        case 'D': cmd_code = IX64_SET_ON_DELTA; args_expected = 2; break;
        default: break;
      }
      break;
    case 'P':
        case 'R': cmd_code = IX64_PRESET_POS; args_expected = 2; break;
    case 'Q':
        case 'U': cmd_code = IX64_QUIT; args_expected = 0; break;
    case 'S':
      switch (buf[1]) {
        case 'I': cmd_code = (IX64_SCAN|IX64_IN); args_expected = 3; break;
        case 'O': cmd_code = (IX64_SCAN|IX64_OUT); args_expected = 3; break;
        case 'P': cmd_code = IX64_STOP; args_expected = 1; break;
        case 'S': cmd_code = IX64_SET_SPEED; args_expected = 2; break;
        case 'T': cmd_code = (IX64_SCAN|IX64_TO); args_expected = 3; break;
        default: break;
      }
      break;
  }
  if (cmd_code == -1) {
    nl_error( 2, "Invalid command in parse_command" );
    return 0;
  }
  if (n_args != args_expected) {
    nl_error( 2, "Incorrect number of args (%d) for command '%c%c': expected %d",
      n_args, buf[0], buf[1], args_expected );
    return 0;
  }
  cmd.dir_scan = cmd_code;
  cmd.drive = n_args > 0 ? args[0] : 0;
  cmd.steps = n_args > 1 ? args[1] : 0;
  cmd.dsteps = n_args > 2 ? args[2] : 0;
  if (cmd.dir_scan == IX64_QUIT) return 1;
  rv = drive_command(&cmd);
  if ( rv != EOK )
    nl_error( 2, "drive_command returned error %d", rv );
  return 0;
}

#define IDXCMDBUFSIZE 256
static int check_command(void) {
  char buf[IDXCMDBUFSIZE];
  for (;;) {
    int rv;
    rv = read(cmd_fd, buf, IDXCMDBUFSIZE-1);
    if ( rv > 0 ) {
      buf[rv] = '\0';
      if (parse_command( buf, rv )) return 1;
    } else if ( rv == 0 ) return 1; // quit command
    else if ( rv == -1 ) {
      if ( errno != EAGAIN )
        nl_error(3, "Received error %d from read() in check_command()", errno );
    } else nl_error(4, "Bad return value [%d] from read in check_command", rv );
    rv = ionotify(cmd_fd, _NOTIFY_ACTION_POLLARM,
        _NOTIFY_COND_INPUT, &cmd_event);
    if (rv == -1) nl_error( 3, "Received error % from iontify in check_command()", errno );
    if (rv == 0) break;
  }
  return 0;
}

/* Return non-zero if the result of the pulse is a command to quit */
int service_pulse(short code, int value ) {
  switch (code) {
    case CMD_PULSE_CODE: return check_command(); break;
    case SCAN_PULSE_CODE: scan_pulse(); break;
    case EXPINT_PULSE_CODE: service_board(value); break;
    default: nl_error( 3, "Invalid pulse_code in service_pulse()" );
  }
  return 0;
}

/* This is the main operational loop */
static void operate( int chid ) {
  struct _pulse pulse;
  int done = 0;

  while ( ! done ) {
    if ( MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == 0 ) {
      done = service_pulse(pulse.code, pulse.value.sival_int);
    } else switch (errno) {
      case EFAULT:
      case EINTR:
      case ESRCH:
      case ETIMEDOUT:
      default:
        nl_error( 2, "Error %d from MsgReceivePulse", errno);
        break;
    }
  }
}

static void close_cmd_fd(void) {
  close(cmd_fd);
}

static void open_cmd_fd( int coid, short code, int value ) {
  int old_response = set_response(0);
  char *cmddev = tm_dev_name("cmd/idx64");
  cmd_fd = tm_open_name(cmddev, NULL, O_RDONLY|O_NONBLOCK);
  set_response(old_response);
  if ( cmd_fd < 0 ) {
    nl_error(3, "Unable to open command channel" );
  }

  /* Initialize cmd event */
  cmd_event.sigev_notify = SIGEV_PULSE;
  cmd_event.sigev_coid = coid;
  cmd_event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
  cmd_event.sigev_code = code;
  cmd_event.sigev_value.sival_int = value;
}

int main( int argc, char **argv ) {
  int resp;
  int coid, chid;

  oui_init_options( argc, argv );
  
  /* Initialize channel to receive pulses*/
  chid = ChannelCreate(0);
  if ( chid == -1 )
    nl_error( 3, "Error %d from ChannelCreate()", errno );
  coid = ConnectAttach( ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
  if ( coid == -1 )
    nl_error( 3, "Error %d from ConnectAttach()", errno );

  open_cmd_fd( coid, CMD_PULSE_CODE, 0 );
  init_boards(coid); /* Configure each board */
  if ( idx64_cfg_string != 0 )
    config_channels( idx64_cfg_string );
  /* boards[0]->chans[1].hysteresis = 100; */
  
  resp = set_response( 1 );
  tm_data = Col_send_init( "Idx64", tm_ptrs, sizeof(tm_ptrs), 0 );
  Col_send_arm( tm_data, coid, SCAN_PULSE_CODE, 0 );
  set_response( resp );
  
  check_command(); // Arm the command channel

  nl_error( 0, "Startup" );
  operate(chid);

  /* cleanup: */
  close_cmd_fd();
  shutdown_boards();
  Col_send_reset( tm_data );
  nl_error( 0, "Terminated" );
  return 0;
}
