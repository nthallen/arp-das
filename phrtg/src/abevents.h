/* Event header for application - AppBuilder 2.03  */

#if defined(__cplusplus)
extern "C" {
#endif

static const ApEventLink_t AbApplLinks[] = {
	{ 3, 0, 0L, 0L, 0L, &Console, NULL, NULL, 0, console_setup, 0, 0, 0, 0, },
	{ 0 }
	};

ApEventLink_t AbInternalLinks[] = {
	{ 3, 0, 0L, 0L, 0L, &Figure, NULL, NULL, 0, plot_figure::Setup, 0, 0, 0, 0, },
	{ 5, 0, 0L, 0L, 0L, &plot_context_menu, NULL, NULL, 0, plot_obj::context_menu_setup, 0, 10, 20, -20, },
	{ 0 }
	};

static const ApEventLink_t AbLinks_Console[] = {
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Console", 1009, console_destroyed, 0, 0, 0, 0, },
	{ 5, 0, 0L, 0L, 0L, &File_Menu, NULL, "file_menu_btn", 2007, NULL, 0, 1, 0, 0, },
	{ 5, 0, 0L, 0L, 0L, &Variable_Menu, NULL, "variable_menu_btn", 2007, NULL, 0, 1, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "ConsoleGroup", 64010, PanelSwitching, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Variables_Tab", 23022, RTG_Variable::TreeSelected, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Graphs_Tab", 23022, plot_obj::TreeSelected, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Graphs_Tab", 23036, plot_obj::TreeColSelect, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Graphs_Tab", 23023, plot_obj::TreeInput, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Window_Name", 4010, Modify_Notify, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Window_Visible", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Windows_Display_Menu", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Window_Display_Name", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Pane_Name", 4010, Modify_Notify, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Pane_Visible", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Pane_Color", 150005, Color_Changed, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Axes_Name", 4010, Modify_Notify, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Axes_Visible", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Axis_Label", 4010, Modify_Notify, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Synch_X_Pane", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Auto_Scale", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Draw_Axis_Primary", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Draw_Axis_Secondary", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Draw_Ticks_Primary", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Draw_Ticks_Secondary", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Draw_Tick_Labels_Secondary", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Draw_Tick_Labels_Primary", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Draw_Label_Primary", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Draw_Label_Secondary", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Log_Scale", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Detrend", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Synch_X_Figure", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Apply_Limits", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Line_Name", 4010, Modify_Notify, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Line_Visible", 2009, Toggle_Activate, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Line_Color", 150005, Color_Changed, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Line_Column", 53015, Numeric_Changed, 0, 0, 0, 0, },
	{ 0 }
	};

static const ApEventLink_t AbLinks_Figure[] = {
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Figure", 1012, plot_figure::Realized, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Figure", 1013, plot_figure::unrealized, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Figure", 1009, plot_figure::destroyed, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Figure", 18017, plot_figure::wmevent, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Figure", 2010, plot_obj::pt_got_focus, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Figure_Div", 10003, plot_figure::divider_resized, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Figure_Div", 44003, plot_figure::divider_drag, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "Figure_Pane", 2010, plot_obj::pt_got_focus, 0, 0, 0, 0, },
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

static const ApEventLink_t AbLinks_plot_context_menu[] = {
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "PlotVisible", 2009, plot_obj::menu_ToggleVisible, 0, 0, 0, 0, },
	{ 8, 0, 0L, 0L, 0L, NULL, NULL, "PlotObjDelete", 2009, plot_obj::menu_Delete, 0, 0, 0, 0, },
	{ 0 }
	};


#if defined(__cplusplus)
}
#endif

