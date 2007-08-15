/* cis_stubs.c
  These are definitions to go in the shared library and resolve
  symbols when programs haven't supplied their own versions.
*/
#include "cmdalgo.h"

char ci_version[] = "";
void cis_initialize(void) {}
void cis_terminate(void) {}
void cis_interfaces(void) {}
int  cmd_batch( char *cmd, int test ) {
  cmd = cmd;
  test = test;
  return 0;
}
void cmd_init(void) {}
void cmd_report(cmd_state *s) {
  s = s;
}
int cmd_check(cmd_state *s) {
  s = s;
  return 0;
}
