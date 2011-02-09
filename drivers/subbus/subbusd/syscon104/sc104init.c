#include "oui.h"
#include "sc104.h"
#include <unistd.h>
#include "nortlib.h"
#include "msg.h"

void sc104_init_options(int argc, char **argv) {
  int optltr;

  optind = OPTIND_RESET;
  opterr = 0;
  while ((optltr = getopt(argc, argv, opt_string)) != -1) {
    switch (optltr) {
      case 'i': process_IRQs(optarg); break;
      case '?':
	nl_error(3, "Unrecognized Option -%c", optopt);
      default:
	break;
    }
  }
}
