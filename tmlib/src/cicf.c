/* cicf.c defines ci_sendfcmd() */
#include <stdarg.h>
#include "nortlib.h"
#include "cmdalgo.h"
char rcsid_cicf_c[] =
  "$Header$";

int ci_sendfcmd(int mode, const char *fmt, ...) {
  va_list arg;
  char cmdbuf[CMD_INTERP_MAX];

  va_start(arg, fmt);
  vsprintf(cmdbuf, fmt, arg);
  va_end(arg);
  return(ci_sendcmd(cmdbuf, mode));
}

/*
=Name ci_sendfcmd(): Send formatted command to Command Server
=Subject Command Server and Client
=Synopsis

#include "nortlib.h"
#include "cmdalgo.h"
int ci_sendfcmd(int mode, char *fmt, ...);

=Description

  ci_sendfcmd() performs the same function as =ci_sendcmd=(),
  but it allows printf()-style formatting in order to send
  variable commands.

=Returns
  
  The same return values as =ci_sendcmd=().

=SeeAlso
=Command Server and Client= functions.

=End
*/
