/* ci_cmdee.c */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "nortlib.h"
#include "nl_assert.h"
#include "tm.h"

/**
 * Initializes a read-only connection to the command server.  Returns the
 * file descriptor or -1. Errors are reported via nl_error with deference to
 * nl_response.
 */
#define CMDEE_BUFSIZE 160
int ci_cmdee_init( char *cmd_node ) {
  char buf[CMDEE_BUFSIZE];
  int fd;

  nl_assert(cmd_node != NULL);
  snprintf(buf, CMDEE_BUFSIZE, "cmd/%s", cmd_node );
  fd = open( tm_dev_name(buf), O_RDONLY );
  if ( fd == -1 ) {
    if (nl_response)
      nl_error( nl_response, "Unable to open tm device %s: %s",
        buf, strerror(errno) );
  }
  return fd;
}
