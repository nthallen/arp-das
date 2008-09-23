#include <errno.h>
#include <string.h>
#include "rdr.h"
/*
 * All of this code is just to pass a single bit of information
 * from the input_thread to the control thread: namely we have
 * reached the end of the input data and it's time to quit.
 *
 * We create a DG_dispatch_client and attach a pulse code similar
 * to DG_tmr.
 *
 * In theory, we could accomplish the same thing by opening our
 * own command interface (tm_dev_name("DG/cmd") and writing the
 * quit code, but that apparently isn't allowed by the resmgr
 * library.
*/
extern "C" {
  static int Rdr_quit_pulse_pulse_func( message_context_t *ctp, int code,
	  unsigned flags, void *handle ) {
    Rdr_quit_pulse *RQP = (Rdr_quit_pulse *)handle;
    RQP->rdr->execute("");
    return 0;
  }
};

Rdr_quit_pulse::Rdr_quit_pulse(Reader *rdr_ptr) : DG_dispatch_client() {
  rdr = rdr_ptr;
}

void Rdr_quit_pulse::attach() {
  pulse_code =
    pulse_attach( rdr->dispatch->dpp, MSG_FLAG_ALLOC_PULSE, 0,
      Rdr_quit_pulse_pulse_func, this );
  if ( pulse_code < 0 )
    nl_error(3, "Error %d from pulse_attach", errno );
  coid = message_connect( rdr->dispatch->dpp, MSG_FLAG_SIDE_CHANNEL );
  if ( coid == -1 )
    nl_error(3, "Error %d from message_connect", errno );
  DG_dispatch_client::attach(rdr->dispatch);
}


int Rdr_quit_pulse::ready_to_quit() {
  if ( pulse_detach(rdr->dispatch->dpp, pulse_code, 0) == -1 ) {
    nl_error( 2, "pulse_detach returned -1" );
  }
  ConnectDetach(coid);
  return 1;
}

Rdr_quit_pulse::~Rdr_quit_pulse() {}

void Rdr_quit_pulse::pulse() {
  MsgSendPulse( coid, getprio(0), pulse_code, 0 );
}
