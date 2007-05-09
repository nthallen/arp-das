/* cictrans.c Defines cic_transmit() */
#include <stdlib.h>
#include "nortlib.h"
#include "cmdalgo.h"
#include "tm.h"

typedef struct cmd_lev {
  struct cmd_lev *prev;
  short int pos; /* offset in buffer */
  cmd_state state;
} cmd_level;

static cmd_level *cur = NULL;
static short int curpos = 0;
static char cmdbuf[CMD_INTERP_MAX];

/* cic_transmit receives commands from the interactive command
   parser and transmits them as required to the command server
   using the nortlib routine ci_sendcmd(). Commands are only
   actually transmitted if the transmit argument is non-zero.
   Since the client and server share source code, it is easy
   for cmdgen to determine whether a particular command will
   result in an action at the far end.
   
   cic_transmit also keeps track of the command parser states,
   recording the commands required to get to a particular
   state. For example, in the HOX command configuration,
   entering "OH\r" will cause the parser to change to a new
   state (submemu) which expects OH commands. This does
   not require transmission, but it does need to be recorded
   because of the new state. Subsequent commands will be
   transmitted prefixed by the "OH\r" until a "\r" is
   entered, returning the parser to the base state.
   
   The command in buf is not ASCIIZ. Spaces may be represented
   by NULs and the terminating newline should be universally
   represented by a NUL. These must be translated back to
   their original representation before actual transmission.
*/
void cic_transmit(char *buf, int n_chars, int transmit) {
  int c;
  cmd_level *cl, *cl_save;

  for (; n_chars > 0; n_chars--) {
	c = *buf++;
	if (c == 0) {
	  if (n_chars == 1) c = '\n';
	  else c = ' ';
	}
	cmdbuf[curpos++] = c;
	if (curpos > CMD_INTERP_MAX)
	  nl_error(3, "Maximum transmissable command length exceeded");
  }
  if (transmit) {
    cmdbuf[curpos] = '\0';
	ci_sendcmd(cmdbuf, 0);
  }
  for (cl = cur; cl != NULL && cmd_check(&cl->state); cl = cl->prev);
  if (cl == NULL) {
	cl = malloc(sizeof(cmd_level));
	if (cl == NULL) nl_error(4, "No memory in cic_transmit");
	cl->prev = cur;
	cl->pos = curpos;
	cmd_report(&cl->state);
	cur = cl;
  } else while (cur != cl) {
	cl_save = cur;
	cur = cur->prev;
	free(cl_save);
  }
  curpos = cur->pos;
}

/*
=Name cic_transmit(): Internal keyboard client send command
=Subject Command Server and Client
=Synopsis

#include "nortlib.h"
void cic_transmit(char *buf, int n_chars, int transmit);

=Description
   cic_transmit() is called by keyboard command clients to
   transmit them as required to the command server
   using the nortlib routine =ci_sendcmd=(). Commands are only
   actually transmitted if the transmit argument is non-zero.
   Since the client and server share source code, it is easy
   for cmdgen to determine whether a particular command will
   result in an action at the far end.<P>
   
   cic_transmit also keeps track of the command parser states,
   recording the commands required to get to a particular
   state. For example, in the HOX command configuration,
   entering "OH\r" will cause the parser to change to a new
   state (submemu) which expects OH commands. This does
   not require transmission, but it does need to be recorded
   because of the new state. Subsequent commands will be
   transmitted prefixed by the "OH\r" until a "\r" is
   entered, returning the parser to the base state.<P>
   
   The command in buf is not ASCIIZ. Spaces may be represented
   by NULs and the terminating newline should be universally
   represented by a NUL. These must be translated back to
   their original representation before actual transmission.<P>
   
   This is an internal function and should not be called
   casually. =ci_sendcmd=() is a better choice for sending
   commands.<P>

=Returns
  Nothing

=SeeAlso
  =ci_sendcmd=(), =Command Server and Client= Functions.

=End
*/
