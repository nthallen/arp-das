/* Event header for application - AppBuilder 2.03  */

#if defined(__cplusplus)
extern "C" {
#endif

static const ApEventLink_t AbApplLinks[] = {
	{ 3, 0, 0L, 0L, 0L, &Console, NULL, NULL, 0, NULL, 0, 0, 0, 0, },
	{ 0 }
	};

ApEventLink_t AbInternalLinks[] = {
	{ 3, 0, 0L, 0L, 0L, &Figure, NULL, NULL, 0, plot_figure::Setup, 0, 0, 0, 0, },
	{ 0 }
	};

static const ApEventLink_t AbLinks_Console[] = {
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Console", 1009, console_destroyed, 0, 0, 0, 0, },
	{ 5, 0, 0L, 0L, 0L, &File_Menu, NULL, "file_menu_btn", 2007, NULL, 0, 1, 0, 0, },
	{ 5, 0, 0L, 0L, 0L, &Variable_Menu, NULL, "variable_menu_btn", 2007, NULL, 0, 1, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "ConsoleGroup", 64010, PanelSwitching, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Variables_Tab", 23022, RTG_Variable::TreeSelected, 0, 0, 0, 0, },
	{ 0 }
	};

static const ApEventLink_t AbLinks_Figure[] = {
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Figure", 1012, plot_figure::Realized, 0, 0, 0, 0, },
	{ 0 }
	};

static const ApEventLink_t AbLinks_File_Menu[] = {
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "menu_open_cmd", 2009, menu_open_cmd, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "menu_close_cmd", 2009, menu_close_cmd, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "menu_file_report", 2009, menu_file_report, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "menu_quit", 2009, menu_quit, 0, 0, 0, 0, },
	{ 0 }
	};

static const ApEventLink_t AbLinks_Variable_Menu[] = {
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "vg_curaxes", 2009, menu_graph_curaxes, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "vg_overlay", 2009, menu_graph_overlay, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "vg_newpane", 2009, menu_graph_newpane, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "vg_newwin", 2009, menu_graph_newwin, 0, 0, 0, 0, },
	{ 0 }
	};


#if defined(__cplusplus)
}
#endif

