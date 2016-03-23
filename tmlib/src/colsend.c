/* colsend.c contains Col_send_init() and Col_send(), since one is
   useless without the other. Col_send_reset() will probably go in
   here also because I'm a little lazy, though it should be used
   together with the other two, it is kinda optional also.
   
   See also collect.h for a description of the message-level protocol.
   
   My strategy with these functions is to be verbose with the init,
   but to be quiet with the other functions. i.e. once a connection
   is established, these functions will not fail in a big way.
   If the DG were to go away, the Sendmx() would fail and I would
   return an error code, but I won't be dying due to nl_response
   for that kind of failure.
*/
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/iomsg.h>
#include <pthread.h>
#include "collect.h"
#include "nortlib.h"
#include "nl_assert.h"
#include "tm.h"
char rcsid_colsend_c[] =
  "$Header$";

/* Establishes connection with collection */
send_id Col_send_init(const char *name, void *data, unsigned short size, int blocking) {
  char data_path[PATH_MAX], *dev_path;
  send_id sender = NULL;
  int fd;
  
  nl_assert( name != 0 && data != 0 );
  if (name[0] == '/') {
    dev_path = name;
  } else {
    snprintf( data_path, PATH_MAX-1, "DG/data/%s", name );
    dev_path = tm_dev_name( data_path );
  }
  fd = open(dev_path, O_WRONLY | (blocking ? 0 : O_NONBLOCK) );
  if ( fd < 0 ) {
    if (nl_response)
      nl_error(nl_response,
        "Col_send_init(%s) failed on open: %s", dev_path, strerror(errno));
    return NULL;
  }
  sender = (send_id)nl_new_memory(sizeof(send_id_struct));
  sender->fd = fd;
  sender->data = data;
  sender->data_size = size;
  sender->err_code = 0;
  sender->armed = blocking ? 0 : 1;
  return sender;
}

int Col_send_arm( send_id sender, int coid, short code, int value ) {
  int policy;
  struct sched_param param;
  nl_assert(sender != 0);
  if (sender->armed == 0) {
    nl_error(nl_response, "Col_send_arm() requires non-blocking option");
    return 1;
  }
  sender->event.sigev_notify = SIGEV_PULSE;
  sender->event.sigev_coid = coid;
  sender->event.sigev_code = code;
  sender->event.sigev_value.sival_int = value;
  sender->event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
  sender->armed = 2;
  return Col_send(sender);
}

/* return 0 on success, non-zero otherwise
   Failure of the send is quiet, but causes an error return.
 */
int Col_send(send_id sender) {
  if ( sender == 0 ) return 1;
  if ( sender->fd >= 0 ) {
    int nb = write(sender->fd, sender->data, sender->data_size);
    if ( nb == -1 ) {
      nl_error(-2,"Col_send() write returned errno %d", errno);
      sender->err_code = errno;
      return 1;
    } else if ( nb == 0 ) {
      return 1;
    }
    if (sender->armed == 2) {
      int rv = ionotify(sender->fd, _NOTIFY_ACTION_POLLARM,
          _NOTIFY_COND_OUTPUT, &(sender->event));
      if (rv == -1) {
	nl_error(MSG_DEBUG, "Col_send() ionotify returned errno %d", errno);
        sender->err_code = errno;
        return 1;
      }
      if (rv & _NOTIFY_COND_OUTPUT) {
	nl_error(MSG_DEBUG, "Col_send() auto-pulsing");
	if ( MsgSendPulse(sender->event.sigev_coid, sender->event.sigev_priority,
              sender->event.sigev_code,
              sender->event.sigev_value.sival_int) == -1)
	  nl_error(4, "Error %d calling MsgSendPulse() in Col_send()", errno);
      } else nl_error(MSG_DEBUG, "Col_send() armed");
    }
  }
  sender->err_code = 0;
  return 0;
}

/**
 * Quietly cleans up the send_id object. Always releases memory,
 * even on error.
 * @return zero on success, non-zero otherwise.
 */
int Col_send_reset(send_id sender) {
  int rc = 0;
  if (sender != 0) {
    if ( close(sender->fd) == -1 ) rc = 1;
    nl_free_memory(sender);
  }
  return rc;
}

/*
=Name Col_send_init(): Initialize communication with collection
=Subject Data Collection
=Subject Startup
=Name Col_send(): Send data to collection
=Subject Data Collection
=Name Col_send_reset(): Terminate communication with collection
=Subject Data Collection

=Synopsis

#include "collect.h"
send_id Col_send_init(const char *name, void *data, unsigned short size);
int Col_send(send_id sender);
int Col_send_reset(send_id sender);

=Description

  These functions define the standard approach for sending data
  to collection. This is necessary whenever an auxilliary program
  is generating data which needs to be fed into the telemetry
  frame.<P>
  
  Col_send_init() initializes the connection, creating a data
  structure to hold all the details. The name string must match the
  name specified in the corresponding 'TM "Receive"' statement in
  the collection program. Data points to where the data will be
  stored, and size specifies the size of the data.<P>
  
  Whenever the data is updated, Col_send() should be called to
  forward the data to collection.<P>
  
  On termination, Col_send_reset() may be called to provide an
  orderly shutdown.

=Returns

  Col_send_init() returns a pointer to a structure which holds
  the state of the connection. This pointer must be passed to
  Col_send() or Col_send_reset(). On error, Col_send_init() will
  return NULL unless =nl_response= indicates a fatal response.<P>
  
  Col_send() returns 0 on success, non-zero otherwise.
  Failure of the send is quiet, but causes an error return.<P>
  
  Col_send_reset() returns 0 on success, non-zero otherwise. It
  does not issue any diagnostic messages, since failure is
  expected to be fairly common (Collection may quit before we get
  around to our termination.)

=SeeAlso

  =Data Collection= functions.

=End
*/
