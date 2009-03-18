/* Y o u r   D e s c r i p t i o n                       */
/*                            AppBuilder Photon Code Lib */
/*                                         Version 2.03  */

/* Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Local headers */
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "proto.h"

#include "msg.h"
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>

/* Application Options string */
const char ApOptions[] =
	AB_OPTIONS "vo:mV"; /* Add your options in the "" */

int (*nl_error)(int level, const char *s, ...) = msg;

int
phrtg_init( int argc, char *argv[] )

	{

	/* eliminate 'unreferenced' warnings */
	argc = argc, argv = argv;
	
	/* Process command line arguments--if any */
	msg_init_options("phrtg", argc, argv);
	nl_error(0, "Starting");
	
    RTG_Variable_MLF::set_default_path("/home/nort/rtgbench");

    /* Load default configuration */
	return( Pt_CONTINUE );

	}


extern "C" {
  static int command_input( int fd, void *data, unsigned mode );
}

static int cmd_fd = -1;

static void close_cmd_fd() {
  if ( cmd_fd >= 0 ) {
	if ( PtAppRemoveFd(NULL, cmd_fd) )
		nl_error(2,"Received error %d from PtAppRemoveFd", errno);
	close(cmd_fd);
	cmd_fd = -1;
  }
}

#define CMD_BUF_SIZE 160
static int command_input( int fd, void *data, unsigned mode ) {
  char cmdbuf[CMD_BUF_SIZE];
  int nb = read( fd, cmdbuf, CMD_BUF_SIZE-1 );
  if (nb>= CMD_BUF_SIZE) nl_error(4,"nb too large from read");
  if ( nb == 0 ) {
	  nl_error( 0, "Received EOF from command channel");
	  close_cmd_fd();
  } else {
	  cmdbuf[nb] = '\0';
    if ( strncmp( "MLF ", cmdbuf, 4 ) == 0 ) {
      char *name = &cmdbuf[4];
      char *s;
      for (s = name; *s && !isspace(*s); ++s );
      if ( *s == '\0' ) {
        nl_error( 2, "MLF without index" );
      } else {
        *s++ = '\0';
        if ( ! isdigit(*s) ) {
          nl_error( 2, "MLF expected digits" );
        } else {
          int index = atoi(s);
          nl_error( 0, "Received MLF %s %d", name, index);
          RTG_Variable_MLF::Incoming( name, index );
        }
      }
    } else nl_error(0,"Unhandled command input: '%s'", cmdbuf);
  }
  return Pt_CONTINUE;
}

int
menu_open_cmd( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo ) {

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	int old_response = set_response(0);
	char *cmddev = tm_dev_name("cmd/phrtg");
	close_cmd_fd();
	cmd_fd = tm_open_name(cmddev,NULL,O_RDONLY);
	set_response(old_response);
	if ( cmd_fd < 0 ) {
		ApError( ABW_Console, errno, "PhRTG", "Unable to open command channel",
		         cmddev );
	} else {
	  if ( PtAppAddFd( NULL, cmd_fd, Pt_FD_READ, command_input, NULL)) {
		  nl_error(2,"Error (%d) calling PtAppAddFd", errno);
		  close_cmd_fd();
	  } else nl_error(0, "menu_open_cmd succeeded");
	}
	return( Pt_CONTINUE );

	}


int
menu_close_cmd( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	close_cmd_fd();
	nl_error(0,"menu_close_cmd");

	return( Pt_CONTINUE );

	}


int
menu_quit( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	nl_error(0,"menu_quit");
	PtExit(0);

	return( Pt_CONTINUE );

	}


int
console_destroyed( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	close_cmd_fd();
	nl_error( 0, "console_destroyed");

	return( Pt_CONTINUE );

	}


int
PanelSwitching( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	PtPanelGroupCallback_t *PGCallback = (PtPanelGroupCallback_t *)cbinfo->cbdata;
	if (strcmp(PGCallback->new_panel, "Variables") &&
		strcmp(PGCallback->new_panel, "Window"))
	  return(Pt_END);
	return( Pt_CONTINUE );

	}


int
menu_graph_curaxes( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;

	return( Pt_CONTINUE );

	}


int
menu_graph_newpane( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	if ( Cur_Figure ) {
	  const char *name = RTG_Variable::Cur_Var ? RTG_Variable::Cur_Var->name : "no_var";
	  new plot_pane(name,Cur_Figure);
	} else menu_graph_newwin( widget, apinfo, cbinfo);

	return( Pt_CONTINUE );

	}


int
menu_graph_newwin( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	const char *name = RTG_Variable::Cur_Var ? RTG_Variable::Cur_Var->name : "no_var";
	new plot_figure(name);
	return( Pt_CONTINUE );

	}


int
menu_graph_overlay( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;

	return( Pt_CONTINUE );

	}


int
menu_file_report( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	plot_figure::Report();
	return( Pt_CONTINUE );

	}

