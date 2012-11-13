/* msg.c */
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include "oui.h"
#include "nortlib.h"
#include "msg.h"
#include "tm.h"

/** Handle -h option
 */
const char *msghdr_init(const char *hdr_default, int argc, char **argv) {
  const char *hdr = hdr_default;
  int c;

  optind = OPTIND_RESET;
  opterr = 0;
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'h': hdr = optarg; break;
      default: break; // could check for errors
    }
  }
  return hdr;
}

#define MSG_MAX_HDR_LEN 10
static char msg_app_hdr[MSG_MAX_HDR_LEN+1];
static int write_to_memo = 1, write_to_stderr = 0, write_to_file = 0;
FILE *memo_fp, *file_fp;

void msg_set_hdr(const char *hdr) {
  strncpy( msg_app_hdr, hdr, MSG_MAX_HDR_LEN );
  msg_app_hdr[MSG_MAX_HDR_LEN] = '\0';
}

/*
<opts> "vo:mVs"

<sort>
  -v add a level of verbosity
  -o <error filename> Write to specified file
  -m write to memo [default]
  -V write to stderr
	-s no message sounds
*/
void msg_init_options(const char *hdr, int argc, char **argv) {
  int c;

  msg_set_hdr(hdr);
  optind = OPTIND_RESET;
  opterr = 0;
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'v': --nl_debug_level; break;
      case 'o':
	file_fp = fopen( optarg, "a" );
	if ( file_fp == NULL ) {
	  fprintf( stderr, "Unable to open output file: '%s'\n", optarg );
	  exit(1);
	}
	write_to_file = 1;
	break;
      case 'm': write_to_memo = 2; break;
      case 'V': write_to_stderr = 1; break;
      case '?':
        fprintf( stderr, "Unrecognized option: '-%c'\n", optopt );
        exit(1);
      default: break; // could check for errors
    }
  }
  if ( ( write_to_file || write_to_stderr ) && write_to_memo == 1 )
    write_to_memo = 0;
  if ( write_to_memo ) {
    memo_fp = fopen( tm_dev_name( "memo" ), "w" );
    if ( memo_fp == NULL ) {
      fprintf( stderr, "Unable to contact memo\n" );
      write_to_stderr = 1;
      write_to_memo = 0;
    }
  }
}

static void write_msg( char *buf, int nb, FILE *fp, char *dest ) {
  int rv = fwrite( buf, 1, nb, fp );
  if ( rv == -1 ) {
    fprintf( stderr, "Memo: error %s writing to %s\n",
       strerror(errno), dest );
  }
  fflush(fp);
}

/**
 * msg() supports the nl_error() interface, but provides
 * a considerable amount of additional functionality to
 * support logging of messages within an application with
 * multiple executables. Through command-line options, msg()
 * can be configured to log to stderr and/or to a log file
 * and/or to the memo application, and adds a timestamp
 * to each message. See nl_error() for definition of the
 * level options.
 * @return the level argument.
 */
int msg( int level, const char *fmt, ...) {
  va_list args;
  int rv;

  va_start(args, fmt);
  rv = msgv( level, fmt, args );
  va_end(args);
  return rv;
}

/**
 * msgv() is a version of the msg() function that
 * takes a va_list for format arguments, allowing
 * more complex reporting functions to built on
 * top of the msg() functionality. Internally
 * msg() calls msgv().
 * @return the level argument.
 */
#define MSG_MAX_INTERNAL 250
int msgv( int level, const char *fmt, va_list args ) {
  char *lvlmsg;
  char msgbuf[MSG_MAX_INTERNAL+2];
  time_t now = time(NULL);
  struct tm *tm = gmtime(&now);
  char *tbuf = asctime(tm);
  int nb;

  switch ( level ) {
    case -1:
    case 0: lvlmsg = ""; break;
    case 1: lvlmsg = "[WARNING] "; break;
    case 2: lvlmsg = "[ERROR] "; break;
    case 3: lvlmsg = "[FATAL] "; break;
    default:
      if ( level >= 4 ) lvlmsg = "[INTERNAL] ";
      else if ( level < nl_debug_level ) return level;
      else lvlmsg = "[DEBUG] ";
      break;
  }
  strncpy(msgbuf, tbuf+11, 9); // index, length of time string
  strncpy( msgbuf+9, lvlmsg, MSG_MAX_INTERNAL-9 );
  nb = 9 + strlen(lvlmsg);
  nb += snprintf( msgbuf+nb, MSG_MAX_INTERNAL-nb, "%s: ", msg_app_hdr );
  // I am guaranteed that we have not yet overflowed the buffer
  nb += vsnprintf( msgbuf+nb, MSG_MAX_INTERNAL-nb, fmt, args );
  if ( nb > MSG_MAX_INTERNAL ) nb = MSG_MAX_INTERNAL;
  if ( msgbuf[nb-1] != '\n' ) msgbuf[nb++] = '\n';
  // msgbuf[nb] = '\0';
  // nb may be as big as MSG_MAX_INTERNAL+1
  // we don't need to transmit the trailing nul

  if ( write_to_memo )
    write_msg( msgbuf, nb, memo_fp ? memo_fp : stderr, "memo" );
  if ( write_to_file ) write_msg( msgbuf, nb, file_fp, "file" );
  if ( write_to_stderr ) write_msg( msgbuf, nb, stderr, "stderr" );
  if ( level >= 4 ) abort();
  if ( level == 3 ) exit(1);
  if ( level == -1 ) exit(0);
  return level;
}
