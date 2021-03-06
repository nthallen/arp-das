#include <sys/neutrino.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "nortlib.h"
#include "parent.h"
#include "oui.h"

static int check_children = 1;
static int saw_timeout = 0;
static int saw_HUP = 0;
static int saw_INT = 0;
int quit_when_childless = 0;
int parent_timeout = 0;
pid_t monitor_pid = 0;

void ChildHandler(int sig) {
  check_children = 1;
}

void AlarmHandler(int sig) {
  saw_timeout = 1;
}
void HUPHandler(int sig) {
  saw_HUP = 1;
}
void INTHandler(int sig) {
  saw_INT = 1;
}

#define MSGSIZE 64

const char *gmt(void) {
  static char buffer[20];
  struct tm *gm;
  time_t now = time(NULL);
  gm = gmtime(&now);
  strftime(buffer, 15, "%H:%M:%S", gm);
  return buffer;
}

int main( int argc, char **argv ) {
  int have_children = 1;
  int chid = ChannelCreate(0);
  int handled_INT = 0;
  int handled_timeout = 0;

  oui_init_options(argc, argv);
  signal(SIGINT, INTHandler);
  signal(SIGCHLD, ChildHandler);
  signal(SIGALRM, AlarmHandler);
  signal(SIGHUP, HUPHandler);
  if (monitor_pid && !parent_timeout)
    parent_timeout = 3;
  if (parent_timeout && !monitor_pid)
    alarm(parent_timeout);

  while (have_children || !quit_when_childless) {
    if ( !check_children) {
      char msg[MSGSIZE];
      int rv = MsgReceive(chid, msg, MSGSIZE, NULL);
      if ( rv == -1 ) {
        if (errno != EINTR) {
          nl_error( 2, "parent: Got a message!" );
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
          // nl_error( 0, "Still have children: none have died" );
          break;
        case -1:
          switch (errno) {
            case ECHILD:
              have_children = 0;
              nl_error( 0, "%s parent: No more children", gmt());
              break;
            case EINTR:
              nl_error( 0, "parent: SIGCHLD in waitpid()" );
              break;
            default:
              nl_error( 2, "parent: Unexpected error from waitpid(): %s",
                strerror(errno));
          }
          break;
        default:
          nl_error( 0, "%s parent: Process %d terminated: status: %04X",
            gmt(), pid, status );
          if (monitor_pid && pid == monitor_pid) {
            alarm(parent_timeout);
          }
          check_children = 1;
          break;
      }
    }
    if ( saw_timeout ) {
      saw_timeout = 0;
      if ( handled_INT )
        nl_error( 3, "%s parent: Timed out waiting for children after INT",
          gmt());
      if ( handled_timeout )
        nl_error( 3,
          "%s parent: Timed out waiting for children after timeout",
          gmt());
      nl_error( 0, "%s parent: Received timeout, calling killpg()", gmt());
      handled_timeout = 1;
      killpg(getpgid(getpid()), SIGHUP);
    }
    if ( saw_HUP) {
      saw_HUP = 0;
      // nl_error( 0, "parent: I saw my own HUP" );
    }
    if ( saw_INT ) {
      saw_INT = 0;
      handled_INT = 1;
      quit_when_childless = 1;
      if ( have_children ) {
        nl_error( 0, "%s parent: Received SIGINT, signaling children",
          gmt());
        check_children = 1;
        killpg(getpgid(getpid()), SIGHUP);
        alarm(3);
      } else {
        nl_error( 0, "%s parent: Received SIGINT", gmt());
      }
    }
  }
  nl_error(0, "%s parent: Shutdown", gmt());
  return 0;
}

