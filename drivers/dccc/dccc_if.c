/* dccc_if.c defines how we get commands */

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "disc_cmd.h"
#include "nortlib.h"
#include "tm.h"

#ifndef DCCC_RESMGR

  static void receive_cmd( int cmd_fd, cmd_t *pcmd ) {
    char tbuf[DCCC_MAX_CMD_BUF+1];

    int nb = read( cmd_fd, tbuf, DCCC_MAX_CMD_BUF );
    if ( nb == -1 )
      nl_error( 3, "Error reading from command interface: %s",
	strerror(errno));
    parse_cmd(tbuf, nb, pcmd);
  }

  void operate(void) {
    int cmd_fd;
    cmd_fd = tm_open_name( tm_dev_name("cmd/dccc"), NULL, O_RDONLY );
    while (!DCCC_Done) {
      cmd_t pcmd;
      receive_cmd(cmd_fd, &pcmd );
    }
  } 

#else /* DCCC_RESMGR */

#endif
