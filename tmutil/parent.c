#include <sys/neutrino.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "nortlib.h"
#include "parent.h"

static int check_children = 1;
static int saw_timeout = 0;
static int saw_HUP = 0;
int quit_when_childless = 0;
int parent_timeout = 0;

void ChildHandler(int sig) {
  check_children = 1;
}

void AlarmHandler(int sig) {
  saw_timeout = 1;
}
void HUPHandler(int sig) {
  saw_HUP = 1;
}

#define MSGSIZE 64

int main( int argc, char **argv ) {
  int have_children = 1;
  int chid = ChannelCreate(0);
  signal(SIGCHLD, ChildHandler);
  signal(SIGALRM, AlarmHandler);
  signal(SIGHUP, HUPHandler);
  if (parent_timeout)
    alarm(parent_timeout);

  while (have_children || !quit_when_childless) {
    if ( !check_children) {
      char msg[MSGSIZE];
      int rv = MsgReceive(chid, msg, MSGSIZE, NULL);
      if ( rv == -1 ) {
	if (errno != EINTR) {
	  nl_error( 2, "Got a message!" );
	}
      }
    }
    if (check_children) {
      int status;
      pid_t pid;
      check_children = 0;
      pid = waitpid( -1, &status, WNOHANG );
      switch (pid) {
	case 0:
	  nl_error( 0, "Still have children: none have died" );
	  break;
	case -1:
	  switch (errno) {
	    case ECHILD:
	      have_children = 0;
	      nl_error( 0, "No more children" );
	      break;
	    case EINTR:
	      nl_error( 0, "SIGCHLD in waitpid()" );
	      break;
	    default:
	      nl_error( 2, "Unexpected error from waitpid(): %s",
		strerror(errno));
	  }
	  break;
	default:
	  nl_error( 0, "Process %d terminated", pid );
	  check_children = 1;
	  break;
      }
    }
    if ( saw_timeout ) {
      saw_timeout = 0;
      nl_error( 0, "Received timeout, calling killpg()" );
      killpg(getpgid(getpid()), SIGHUP);
    }
    if ( saw_HUP) {
      saw_HUP = 0;
      nl_error( 0, "I saw my own HUP" );
    }
  }
  nl_error(0, "Shutdown" );
  return 0;
}

