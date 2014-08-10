#include <errno.h>
#include <sys/select.h>
#include <atomic.h>
#include "Selector.h"
#include "nortlib.h"
#include "nl_assert.h"

Selector::Selector() {
  children_changed = false;
  gflags = 0;
}

/**
 * Actually does nothing.
 */
Selector::~Selector() {
}

/**
 * Adds the Selectee as a child of the Selector. This adds the Selectee's fd
 * to the list of fds in the Selector's select() call, depending on the
 * Selectee's flags.
 */
void Selector::add_child(Selectee *P) {
  if (find_child_by_fd(P->fd) == S.end() ) {
    S.push_back(P);
    P->Stor = this;
    children_changed = true;
  } else {
    nl_error( 4, "fd %d already inserted in Selector::add_child", P->fd );
  }
}

SelecteeVec::iterator Selector::find_child_by_fd(int fd) {
  SelecteeVec::iterator pos;
  for ( pos = S.begin(); pos != S.end(); ++pos ) {
    Selectee *P;
    P = *pos;
    if (P->fd == fd) return pos;
  }
  return S.end();
}

/**
 * Removes Selectee from the Selector and deletes the Selectee. Note that this
 * is for use in a special case where the fd is being closed and the child
 * needs to be deleted, but the caller does not know the mapping from the fd
 * to the Selectee. In the usual case, the Selectee is deleted by its owner
 * after the event_loop has exited, and it is unnecessary to explicitly
 * remove the Selectees as children of the Selector.
 */
void Selector::delete_child(int fd_in) {
  SelecteeVec::iterator pos;
  Selectee *P;

  pos = find_child_by_fd(fd_in);
  if ( pos == S.end() )
    nl_error( 4,
      "Selectee not found for fd %d in Selector::delete_child()", fd_in );
  P = *pos;
  nl_assert( fd_in == P->fd );
  S.erase(pos);
  children_changed = true;
  delete P;
}

/**
 * Useful when we know the fd but not the Selectee.
 * \returns 0 on success, 1 if fd_in is not found.
 */
int Selector::update_flags(int fd_in, int flag) {
  SelecteeVec::const_iterator pos;
  pos = find_child_by_fd(fd_in);
  if ( pos != S.end() ) {
    Selectee *P = *pos;
    P->flags = flag;
    children_changed = true;
    return 0;
  } else {
    return 1;
  }
}

/**
 * Sets a bit in the global flags word. Selectees can set
 * a corresponding bit in their flags word to request
 * notification when the bit gets set. The function
 * Selector::gflag(gflag_index) returns the bit that
 * corresponds to set_gflag(gflag_index). gflag_index
 * can take on values from 0 to 8*sizeof(int)-4.
 */
void Selector::set_gflag( unsigned gflag_index ) {
  nl_assert(gflag_index+4 < sizeof(int)*8 );
  // gflags |= gflag(gflag_index);
  atomic_set((unsigned *)&gflags, gflag(gflag_index));
}

/**
 * Loops waiting on select(), using the fds and flags of
 * each Selectee registered via add_child(). When select()
 * indicates that an fd is ready, the corresponding Selectee's
 * ProcessData() method is invoked with the flag value indicating
 * what action is ready.
 */
void Selector::event_loop() {
  int keep_going = 1;
  int width = 0;
  int rc;
  fd_set readfds, writefds, exceptfds;
  
  while (keep_going) {
    TimeoutAccumulator to;
    SelecteeVec::const_iterator Sp;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    to.Set_Min(GetTimeout());
    children_changed = false;
    for ( Sp = S.begin(); Sp != S.end(); ++Sp ) {
      Selectee *P = *Sp;
      if (P->flags & gflags) {
        int flag = P->flags & gflags;
        atomic_clr((unsigned *)&gflags, flag);
        P->ProcessData(flag);
      }
    }
    for ( Sp = S.begin(); Sp != S.end(); ++Sp ) {
      Selectee *P = *Sp;
      if (P->flags & Sel_Read) FD_SET(P->fd, &readfds);
      if (P->flags & Sel_Write) FD_SET(P->fd, &writefds);
      if (P->flags & Sel_Except) FD_SET(P->fd, &exceptfds);
      if (P->flags & Sel_Timeout) to.Set_Min( P->GetTimeout() );
      if (width <= P->fd) width = P->fd+1;
    }
    rc = select(width, &readfds, &writefds, &exceptfds, to.timeout_val());
    if ( rc == 0 ) {
      if ( ProcessTimeout() )
        keep_going = 0;
      for ( Sp = S.begin(); Sp != S.end(); ++Sp ) {
        Selectee *P = *Sp;
        if ((P->flags & Sel_Timeout) && P->ProcessData(Sel_Timeout))
          keep_going = 0;
      }
    } else if ( rc < 0 ) {
      if ( errno == EINTR ) keep_going = 0;
      else nl_error(3, "Unexpected error: %d", errno);
    } else {
      for ( Sp = S.begin(); Sp != S.end(); ++Sp ) {
        Selectee *P = *Sp;
        int flags = 0;
        if ( (P->flags & Sel_Read) && FD_ISSET(P->fd, &readfds) )
          flags |= Sel_Read;
        if ( (P->flags & Sel_Write) && FD_ISSET(P->fd, &writefds) )
          flags |= Sel_Write;
        if ( (P->flags & Sel_Except) && FD_ISSET(P->fd, &exceptfds) )
          flags |= Sel_Except;
        if ( flags ) {
          if ( P->ProcessData(flags) )
            keep_going = 0;
          if (children_changed) break; // Changes can occur during ProcessData
        }
      }
    }
  }
}

/**
 * Virtual method called whenever select() returns 0. The default does nothing,
 * but it can be overridden.
 * @return non-zero if the event loop should terminate.
 */
int Selector::ProcessTimeout() { return 0; }

/**
 * Virtual method to allow Selector to bid on the select() timeout
 * along with the Selectee children. The minimum timeout value is used.
 * @return a Timeout * indicating the requested timeout value or NULL.
 */
Timeout *Selector::GetTimeout() { return NULL; }

extern "C" {
  char libtmpp_is_present(void) { return '\0'; }
}
