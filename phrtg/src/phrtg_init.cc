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
#include "nl_assert.h"

/* Application Options string */
const char ApOptions[] =
	AB_OPTIONS "vo:mV"; /* Add your options in the "" */

int (*nl_error)(int level, const char *s, ...) = msg;
static void open_cmd_fd();

int phrtg_init( int argc, char *argv[] ) {
  /* Process command line arguments--if any */
  msg_init_options("phrtg", argc, argv);
  nl_error(0, "Starting");
	
  RTG_Variable_MLF::set_default_path("/home/nort/PhRTGbench2");
  open_cmd_fd();

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

static void open_cmd_fd() {
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
	} else nl_error(-2, "menu_open_cmd succeeded");
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
          nl_error( -2, "Received MLF %s %d", name, index);
          RTG_Variable_MLF::Incoming( name, index );
        }
      }
    } else nl_error(2,"Unhandled command input: '%s'", cmdbuf);
  }
  //plot_obj::render_all();
  return Pt_CONTINUE;
}

int menu_open_cmd( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  open_cmd_fd();
  return( Pt_CONTINUE );
}


int menu_close_cmd( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  close_cmd_fd();
  nl_error(-2,"menu_close_cmd");
  return( Pt_CONTINUE );
}

int menu_quit( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  nl_error(0,"menu_quit");
  PtExit(0);
  return( Pt_CONTINUE );
}

int console_destroyed( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  close_cmd_fd();
  nl_error( 0, "console_destroyed");
  PtExit(0);
  return( Pt_CONTINUE );
}

int PanelSwitching( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo;
  PtPanelGroupCallback_t *PGCallback =
		(PtPanelGroupCallback_t *)cbinfo->cbdata;
  if (strcmp(PGCallback->new_panel, "Window") == 0) {
    if (Current::Figure == NULL) return Pt_END;
    Current::Tab = Tab_Figure;
    Current::Figure->Update_Window_Tab();
  } else if (strcmp(PGCallback->new_panel, "X") == 0) {
    if (Current::Pane == NULL) return Pt_END;
    Current::Tab = Tab_X;
    PtReparentWidget(ABW_Axis_Pane, ABW_X_Tab);
    Current::Pane->Update_Axis_Pane();
    if (Current::Axes) Current::Axes->Update_Axis_Pane(Axis_X);
    else plot_axis::Clear_Axis_Pane();
  } else if (strcmp(PGCallback->new_panel, "Y") == 0) {
    if (Current::Pane == NULL) return Pt_END;
    Current::Tab = Tab_Y;
    PtReparentWidget(ABW_Axis_Pane, ABW_Y_Tab);
    Current::Pane->Update_Axis_Pane();
    if (Current::Axes) Current::Axes->Update_Axis_Pane(Axis_Y);
    else plot_axis::Clear_Axis_Pane();
  } else if (strcmp(PGCallback->new_panel, "Line") == 0) {
    if (Current::Line == NULL) return Pt_END;
    Current::Tab = Tab_Line;
    Current::Line->Update_Line_Tab();
  }
  return Pt_CONTINUE;
}

int menu_graph_curaxes( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  if (Current::Axes) {
  	if (Current::Variable != NULL) {
  	  plot_data *graph =
  	    Current::Axes->CreateGraph(Current::Variable);
  	  graph->got_focus(focus_from_user);
  	  plot_obj::render_all();
    } else nl_error(2, "No Current Variable defined");
  } else {
    nl_error(-2, "m_g_curaxes: No Current axes: calling m_g_overlay");
    menu_graph_overlay( widget, apinfo, cbinfo);
  }
  return( Pt_CONTINUE );
}

int menu_graph_overlay( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  if (Current::Pane != NULL) {
    if (Current::Variable != NULL) {
      plot_axes *ax = Current::Pane->CreateGraph(Current::Variable);
      ax->got_focus(focus_from_user);
      plot_obj::render_all();
    } else nl_error(2, "No current variable defined");
  } else {
  	nl_error(-2,"m_g_overlay: no current pane, calling m_g_newpane");
  	menu_graph_newpane(widget,apinfo,cbinfo);
  }
  return Pt_CONTINUE;
}


int menu_graph_newpane( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  if ( Current::Figure ) {
  	if (Current::Variable) {
      // Current::Figure->CreateGraph(Current::Variable);
  	  plot_pane *pane = Current::Figure->CreateGraph(Current::Variable);
  	  pane->got_focus(focus_from_user);
  	  plot_obj::render_all();
  	} else nl_error( 2, "No Current Variable defined");
  } else {
  	nl_error(-2, "m_g_newpane: No current figure, calling m_g_newwin");
  	menu_graph_newwin( widget, apinfo, cbinfo);
  }
  return( Pt_CONTINUE );
}

int menu_graph_newwin( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  if (Current::Variable != NULL) {
    const char *name = Current::Variable->name;
    plot_figure *fig = new plot_figure(name);
    fig->CreateGraph(Current::Variable);
    fig->got_focus(focus_from_user);
    plot_obj::render_all();
  } else nl_error(2, "No current variable defined");
  return Pt_CONTINUE;
}

int menu_file_report( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  plot_figure::Report();
  return( Pt_CONTINUE );
}


int console_setup( PtWidget_t *link_instance, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
	PtTreeColumnAttributes_t col_attrs[] = {
		{ NULL, 0, 0, 0 },
		{ NULL, 0, 0, 0 }
	};
	/* eliminate 'unreferenced' warnings */
	link_instance = link_instance, apinfo = apinfo, cbinfo = cbinfo;
	PtWidget_t *graphs = ApGetWidgetPtr(link_instance, ABN_Graphs_Tab);
	PtSetResource(graphs, Pt_ARG_TREE_COLUMN_ATTR, col_attrs, 2 );
	/* Set pallettes for color widgets */
	return( Pt_CONTINUE );
}
