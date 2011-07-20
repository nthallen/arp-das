/** \file trend.cc
 * Support for trending graphs
 */
#include "ablibs.h"
#include "phrtg.h"
#include "nortlib.h"

void RTG_Variable_Trend::Incoming(const char *cmd) {
  nl_error(0, "RTG_Variable_Trend::Income(\"%s\")", cmd);
}
