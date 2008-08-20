/* msg.c */
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include "oui.h"
#include "nortlib.h"

/** Handle -h option
 */
char *msghdr_init(char *hdr_default, int argc, char **argv) {
  char *hdr = hdr_default;
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

void msg_set_hdr(char *hdr) {
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
void msg_init_options(char *hdr, int argc, char **argv) {
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
      case 's': break;
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
      exit(1);
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

#define MSG_MAX_INTERNAL 250
int msg( int level, char *fmt, ... ) {
  va_list args;
  char *lvlmsg;
  char msgbuf[MSG_MAX_INTERNAL+2];
  time_t now = time(NULL);
  char *tbuf = asctime(gmtime(&now));
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
  va_start(args, fmt);
  nb += vsnprintf( msgbuf+nb, MSG_MAX_INTERNAL-nb, fmt, args );
  va_end(args);
  if ( nb > MSG_MAX_INTERNAL ) nb = MSG_MAX_INTERNAL;
  if ( msgbuf[nb-1] != '\n' ) msgbuf[nb++] = '\n';
  // msgbuf[nb] = '\0';
  // nb may be as big as MSG_MAX_INTERNAL+1
  // we don't need to transmit the trailing nul

  if ( write_to_memo ) write_msg( msgbuf, nb, memo_fp, "memo" );
  if ( write_to_file ) write_msg( msgbuf, nb, file_fp, "file" );
  if ( write_to_stderr ) write_msg( msgbuf, nb, stderr, "stderr" );
  return level;
}
