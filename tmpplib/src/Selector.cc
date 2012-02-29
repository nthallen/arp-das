#include <errno.h>
#include <sys/select.h>
#include "Selector.h"
#include "nortlib.h"
#include "nl_assert.h"

Selector::Selector() {
  children_changed = false;
  gflags = 0;
}

Selector::~Selector() {
}

void Selector::add_child(Selectee *P) {
  if (S.find(P->fd) == S.end() ) {
    S[P->fd] = P;
    P->Stor = this;
    children_changed = true;
  } else {
    nl_error( 4, "fd %d already inserted in Selector::add_child", P->fd );
  }
}

/**
 * Removes Selectee from the Selector and deletes the Selectee.
 */
void Selector::delete_child(int fd_in) {
  SelecteeMap::iterator pos;
  Selectee *P;
  pos = S.find(fd_in);
  if ( pos == S.end() )
    nl_error( 4, "Selectee not found for fd %d in Selector::delete_child()", fd_in );
  P = pos->second;
  nl_assert( fd_in == P->fd );
  if ( S.erase(fd_in) == 0 ) {
    nl_error( 4, "fd %d not found in Selector::delete_child()", P->fd);
  }
  children_changed = true;
  delete P;
}

/**
 * Useful when we know the fd but not the Selectee.
 * \returns 0 on success, 1 if fd_in is not found.
 */
int Selector::update_flags(int fd_in, int flag) {
  SelecteeMap::const_iterator pos;
  pos = S.find(fd_in);
  if ( pos != S.end() ) {
    Selectee *P = pos->second;
    P->flags = flag;
    children_changed = true;
    return 0;
  } else {
    return 1;
  }
}

void Selector::set_gflag( unsigned gflag_index ) {
  nl_assert(gflag_index+4 < sizeof(int)*8 );
  gflags |= gflag(gflag_index);
}

void Selector::event_loop() {
  int keep_going = 1;
  int width = 0;
  int rc;
  fd_set readfds, writefds, exceptfds;
  
  while (keep_going) {
    TimeoutAccumulator to;
    SelecteeMap::const_iterator Sp;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    to.Set_Min(GetTimeout());
    children_changed = false;
    for ( Sp = S.begin(); Sp != S.end(); ++Sp ) {
      Selectee *P = Sp->second;
      if (P->flags & gflags) {
	P->ProcessData(P->flags & gflags);
	gflags &= ~(P->flags & gflags);
      }
    }
    for ( Sp = S.begin(); Sp != S.end(); ++Sp ) {
      Selectee *P = Sp->second;
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
        Selectee *P = Sp->second;
        if ((P->flags & Sel_Timeout) && P->ProcessData(Sel_Timeout))
          keep_going = 0;
      }
    } else if ( rc < 0 ) {
      if ( errno == EINTR ) keep_going = 0;
      else nl_error(3, "Unexpected error: %d", errno);
    } else {
      for ( Sp = S.begin(); Sp != S.end(); ++Sp ) {
        Selectee *P = Sp->second;
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

int Selector::ProcessTimeout() { return 0; }
Timeout *Selector::GetTimeout() { return NULL; }
