/* Define header for application - AppBuilder 2.03  */

#if defined(__cplusplus)
extern "C" {
#endif

/* Internal Module Links */
extern const int ABI_Figure;
#define ABM_Figure                           &AbInternalLinks[ABI_Figure]
extern const int ABI_plot_context_menu;
#define ABM_plot_context_menu                &AbInternalLinks[ABI_plot_context_menu]

/* 'Figure' Window link */
extern const int ABN_Figure;
#define ABW_Figure                           AbGetABW( ABN_Figure )
extern const int ABN_Figure_Div;
#define ABW_Figure_Div                       AbGetABW( ABN_Figure_Div )

/* 'Console' Window link */
extern const int ABN_Console;
#define ABW_Console                          AbGetABW( ABN_Console )
extern const int ABN_file_menu_btn;
#define ABW_file_menu_btn                    AbGetABW( ABN_file_menu_btn )
extern const int ABN_variable_menu_btn;
#define ABW_variable_menu_btn                AbGetABW( ABN_variable_menu_btn )
extern const int ABN_Zoom_menu_btn;
#define ABW_Zoom_menu_btn                    AbGetABW( ABN_Zoom_menu_btn )
extern const int ABN_ConsoleGroup;
#define ABW_ConsoleGroup                     AbGetABW( ABN_ConsoleGroup )
extern const int ABN_Variables_Tab;
#define ABW_Variables_Tab                    AbGetABW( ABN_Variables_Tab )
extern const int ABN_Graphs_Tab;
#define ABW_Graphs_Tab                       AbGetABW( ABN_Graphs_Tab )
extern const int ABN_Window_Tab;
#define ABW_Window_Tab                       AbGetABW( ABN_Window_Tab )
extern const int ABN_Figure_Name;
#define ABW_Figure_Name                      AbGetABW( ABN_Figure_Name )
extern const int ABN_Figure_Visible;
#define ABW_Figure_Visible                   AbGetABW( ABN_Figure_Visible )
extern const int ABN_Windows_Display_Menu;
#define ABW_Windows_Display_Menu             AbGetABW( ABN_Windows_Display_Menu )
extern const int ABN_Window_Display_Name;
#define ABW_Window_Display_Name              AbGetABW( ABN_Window_Display_Name )
extern const int ABN_X_Tab;
#define ABW_X_Tab                            AbGetABW( ABN_X_Tab )
extern const int ABN_Axis_Pane;
#define ABW_Axis_Pane                        AbGetABW( ABN_Axis_Pane )
extern const int ABN_Pane_Name;
#define ABW_Pane_Name                        AbGetABW( ABN_Pane_Name )
extern const int ABN_Pane_Visible;
#define ABW_Pane_Visible                     AbGetABW( ABN_Pane_Visible )
extern const int ABN_Pane_Color;
#define ABW_Pane_Color                       AbGetABW( ABN_Pane_Color )
extern const int ABN_Axes_Name;
#define ABW_Axes_Name                        AbGetABW( ABN_Axes_Name )
extern const int ABN_Axes_Visible;
#define ABW_Axes_Visible                     AbGetABW( ABN_Axes_Visible )
extern const int ABN_Axis_Label;
#define ABW_Axis_Label                       AbGetABW( ABN_Axis_Label )
extern const int ABN_Synch_X_Pane;
#define ABW_Synch_X_Pane                     AbGetABW( ABN_Synch_X_Pane )
extern const int ABN_Auto_Scale;
#define ABW_Auto_Scale                       AbGetABW( ABN_Auto_Scale )
extern const int ABN_Draw_Axis_Primary;
#define ABW_Draw_Axis_Primary                AbGetABW( ABN_Draw_Axis_Primary )
extern const int ABN_Draw_Axis_Secondary;
#define ABW_Draw_Axis_Secondary              AbGetABW( ABN_Draw_Axis_Secondary )
extern const int ABN_Draw_Ticks_Primary;
#define ABW_Draw_Ticks_Primary               AbGetABW( ABN_Draw_Ticks_Primary )
extern const int ABN_Draw_Ticks_Secondary;
#define ABW_Draw_Ticks_Secondary             AbGetABW( ABN_Draw_Ticks_Secondary )
extern const int ABN_Draw_Tick_Labels_Secondary;
#define ABW_Draw_Tick_Labels_Secondary       AbGetABW( ABN_Draw_Tick_Labels_Secondary )
extern const int ABN_Draw_Tick_Labels_Primary;
#define ABW_Draw_Tick_Labels_Primary         AbGetABW( ABN_Draw_Tick_Labels_Primary )
extern const int ABN_Draw_Label_Primary;
#define ABW_Draw_Label_Primary               AbGetABW( ABN_Draw_Label_Primary )
extern const int ABN_Draw_Label_Secondary;
#define ABW_Draw_Label_Secondary             AbGetABW( ABN_Draw_Label_Secondary )
extern const int ABN_Log_Scale;
#define ABW_Log_Scale                        AbGetABW( ABN_Log_Scale )
extern const int ABN_Detrend;
#define ABW_Detrend                          AbGetABW( ABN_Detrend )
extern const int ABN_Synch_X_Figure;
#define ABW_Synch_X_Figure                   AbGetABW( ABN_Synch_X_Figure )
extern const int ABN_Limit_Min;
#define ABW_Limit_Min                        AbGetABW( ABN_Limit_Min )
extern const int ABN_Limit_Max;
#define ABW_Limit_Max                        AbGetABW( ABN_Limit_Max )
extern const int ABN_Apply_Limits;
#define ABW_Apply_Limits                     AbGetABW( ABN_Apply_Limits )
extern const int ABN_PSD;
#define ABW_PSD                              AbGetABW( ABN_PSD )
extern const int ABN_Phase;
#define ABW_Phase                            AbGetABW( ABN_Phase )
extern const int ABN_Invert;
#define ABW_Invert                           AbGetABW( ABN_Invert )
extern const int ABN_Trend;
#define ABW_Trend                            AbGetABW( ABN_Trend )
extern const int ABN_TrendScale;
#define ABW_TrendScale                       AbGetABW( ABN_TrendScale )
extern const int ABN_Y_Tab;
#define ABW_Y_Tab                            AbGetABW( ABN_Y_Tab )
extern const int ABN_Line_Tab;
#define ABW_Line_Tab                         AbGetABW( ABN_Line_Tab )
extern const int ABN_Line_Name;
#define ABW_Line_Name                        AbGetABW( ABN_Line_Name )
extern const int ABN_Line_Visible;
#define ABW_Line_Visible                     AbGetABW( ABN_Line_Visible )
extern const int ABN_Line_Type;
#define ABW_Line_Type                        AbGetABW( ABN_Line_Type )
extern const int ABN_Line_Color;
#define ABW_Line_Color                       AbGetABW( ABN_Line_Color )
extern const int ABN_Line_Variable_Name;
#define ABW_Line_Variable_Name               AbGetABW( ABN_Line_Variable_Name )
extern const int ABN_Line_Column;
#define ABW_Line_Column                      AbGetABW( ABN_Line_Column )

#define AbGetABW( n ) ( AbWidgets[ n ].wgt )

/* 'File_Menu' Menu link */
extern const int ABN_File_Menu;
extern const int ABN_menu_open_cmd;
extern const int ABN_menu_close_cmd;
extern const int ABN_menu_file_report;
extern const int ABN_menu_save_cfg;
extern const int ABN_menu_quit;

/* 'Variable_Menu' Menu link */
extern const int ABN_Variable_Menu;
extern const int ABN_vg_curaxes;
extern const int ABN_vg_overlay;
extern const int ABN_vg_newpane;
extern const int ABN_vg_newwin;

/* 'plot_context_menu' Menu link */
extern const int ABN_plot_context_menu;
extern const int ABN_PlotVisible;
extern const int ABN_PlotObjDelete;

#define AB_OPTIONS "s:"

#if defined(__cplusplus)
}
#endif

