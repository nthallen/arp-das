#include <errno.h>
#include "DG_Resmgr.h"
#include "nortlib.h"
#include "nl_assert.h"

/** DG_Resmgr.h Framework for 
*/

DG_dispatch_client::DG_dispatch_client() {
  dispatch = NULL;
}

DG_dispatch_client::~DG_dispatch_client() {
  if ( dispatch != NULL )
    detach();
}

void DG_dispatch_client::attach(DG_dispatch *disp) {
  // assert( disp != NULL );
  dispatch = disp;
  dispatch->client_add(this);
}

void DG_dispatch_client::detach() {
  //assert(dispatch != NULL);
  dispatch->client_rm(this);
  dispatch = NULL;
}

DG_dispatch::DG_dispatch() {
    dpp = dispatch_create();
  if ( dpp == NULL )
    nl_error( 3, "Failed to allocate dispatch handle." );
  quit_received = 0;
}

void DG_dispatch::ready_to_quit() {
    quit_received = 1;
}

DG_dispatch::~DG_dispatch() {
    if ( single_ctp != NULL )
        dispatch_context_free(single_ctp);
  dispatch_destroy(dpp);
}

void DG_dispatch::Loop() {
    single_ctp = dispatch_context_alloc(dpp);
  if ( single_ctp == NULL )
    nl_error(3, "dispatch_context_alloc failed: errno %d", errno );
  dispatch_context_t *ctp = single_ctp;
  while (1) {
    ctp = dispatch_block(ctp);
    if ( ctp == NULL )
      nl_error( 3, "Block error: %d", errno );
    dispatch_handler(ctp);
    if ( quit_received && ctp->resmgr_context.rcvid == 0
      && all_closed() )
      break;
  }
}

int DG_dispatch::all_closed() {
  int ready = 1;
  std::list<DG_dispatch_client *>::iterator pos;
  for ( pos = clients.begin(); pos != clients.end(); ) {
    if ( (*pos)->ready_to_quit() ) clients.remove(*pos++);
    else {
      ready = 0;
      ++pos;
    }
  }
  return ready;
}

void DG_dispatch::client_add(DG_dispatch_client *clt) {
  //assert(clt != NULL);
  clients.push_back(clt);
}

void DG_dispatch::client_rm(DG_dispatch_client *clt) {
  //assert(clt != NULL);
  clients.remove(clt);
}
