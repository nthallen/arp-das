#ifndef RESMGR_H_
#define RESMGR_H_

#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/resmgr.h>
#include <list>

class DG_dispatch;

/**
  DG_dispatch_client is a virtual base class for clients of DG_dispatch. The main service provided is quit coordination.
  ready_to_quit() virtual function should return true if client is ready to quit. This will remove the client from the
  DG_dispatch's list, so a subsequent detach() is unnecessary. Hence ready_to_quit() should set the dispatch member to
  NULL before returning a non-zero value.
  As a rule, clients should shut down all of their functions prior to returning. For devices, that means resmgr_detach()
  
  As written, this is probably a bad design, since DG_dispatch::client_add is called with a partially constructed client.
  In a multi-threaded environment, there is a potential for harm.
*/
class DG_dispatch_client {
  public:
    DG_dispatch_client();
    virtual ~DG_dispatch_client(); // calls detach() if necessary
    void attach(DG_dispatch *disp); // add to dispatch list
    void detach(); // remove from dispatch list
    virtual int ready_to_quit()= 0;
    DG_dispatch *dispatch;
};

class DG_dispatch {
  public:
    dispatch_t *dpp;
    DG_dispatch();
    ~DG_dispatch();
    void client_add(DG_dispatch_client *clt);
    void client_rm(DG_dispatch_client *clt);
    void Loop();
    void ready_to_quit();
  private:
    int quit_received;
    int all_closed();
    dispatch_context_t *single_ctp;
    std::list<DG_dispatch_client *> clients;
};

#endif /*RESMGR_H_*/
