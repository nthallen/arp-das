/* msg.c */
#include "oui.h"

/** Handle -h option
 */
char *msghdr_init(char *hdr_default, int argc, char **argv) {
  char *hdr = hdr_default;
  int c;

  optind = 0;
  opterr = OPTIND_RESET;
  while ((c = getopt(argc, argv, opt_string)) != -1) {
  	switch (c) {
  	  case 'h': hdr = optarg; break;
      default: break; // could check for errors
    }
  }
  return hdr;
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

  optind = 0;
  opterr = OPTIND_RESET;
  while ((c = getopt(argc, argv, opt_string)) != -1) {
  	switch (c) {
  	  case 'v': --nl_debug_level; break;
      case 'o': break;
      case 'm': break;
      case 'V': break;
      case 's': break;
      default: break; // could check for errors
    }
  }
}

int msg(int level, char *s, ...) {
}
