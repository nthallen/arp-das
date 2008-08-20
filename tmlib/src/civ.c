#include <stdio.h>
#include "tm.h"

void ci_report_version(void) {
  printf( "%s", ci_version );
  exit(0);
}
