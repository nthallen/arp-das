/* Link header for application - AppBuilder 2.03  */

#if defined(__cplusplus)
extern "C" {
#endif

extern ApContext_t AbContext;

ApWindowLink_t Console = {
	"Console.wgtw",
	&AbContext,
	AbLinks_Console, 0, 13
	};

ApWindowLink_t Figure = {
	"Figure.wgtw",
	&AbContext,
	AbLinks_Figure, 44, 8
	};

static ApItem_t ApItems_File_Menu[ 5 ] = {
	{ 1, 1, 0, NULL, 0, "menu_open_cmd", "Open Command Channel", NULL },
	{ 1, 1, 0, NULL, 0, "menu_close_cmd", "Close Command Channel", NULL },
	{ 1, 1, 0, NULL, 0, "menu_file_report", "Report", NULL },
	{ 1, 1, 0, NULL, 0, "menu_quit", "Quit", NULL },
	{ 0, 0, NULL, NULL, 0, NULL, NULL, NULL } };

ApMenuLink_t File_Menu = {
	"File_Menu",
	"",
	NULL,
	NULL,
	2,
	ApItems_File_Menu,
	& AbContext,
	AbLinks_File_Menu,
	47, 4, 4
	};

static ApItem_t ApItems_Variable_Menu[ 5 ] = {
	{ 1, 1, 0, NULL, 0, "vg_curaxes", "graph on current axes", NULL },
	{ 1, 1, 0, NULL, 0, "vg_overlay", "graph overlay", NULL },
	{ 1, 1, 0, NULL, 0, "vg_newpane", "graph in new pane", NULL },
	{ 1, 1, 0, NULL, 0, "vg_newwin", "graph in new window", NULL },
	{ 0, 0, NULL, NULL, 0, NULL, NULL, NULL } };

ApMenuLink_t Variable_Menu = {
	"Variable_Menu",
	"",
	NULL,
	NULL,
	2,
	ApItems_Variable_Menu,
	& AbContext,
	AbLinks_Variable_Menu,
	52, 4, 4
	};

static ApItem_t ApItems_plot_context_menu[ 3 ] = {
	{ 1, 4, 0, NULL, 0, "PlotVisible", "Visible", NULL },
	{ 1, 1, 0, NULL, 0, "PlotObjDelete", "Delete", NULL },
	{ 0, 0, NULL, NULL, 0, NULL, NULL, NULL } };

ApMenuLink_t plot_context_menu = {
	"plot_context_menu",
	"",
	NULL,
	NULL,
	2,
	ApItems_plot_context_menu,
	& AbContext,
	AbLinks_plot_context_menu,
	57, 2, 2
	};


#if defined(__cplusplus)
}
#endif

