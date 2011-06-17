/* Import (extern) header for application - AppBuilder 2.03  */

#if defined(__cplusplus)
extern "C" {
#endif

#include "abdefine.h"

extern ApEventLink_t AbInternalLinks[];

extern ApWindowLink_t Figure;
extern ApWindowLink_t Console;
extern ApWidget_t AbWidgets[ 49 ];

extern ApMenuLink_t File_Menu;
extern ApMenuLink_t Variable_Menu;
extern ApMenuLink_t plot_context_menu;

#if defined(__cplusplus)
}
#endif


#ifdef __cplusplus
int phrtg_init( int argc, char **argv );
int console_setup( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_open_cmd( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_close_cmd( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int console_destroyed( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int PanelSwitching( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_graph_curaxes( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_graph_overlay( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_graph_newpane( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_graph_newwin( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_file_report( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int Modify_Notify( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int Color_Changed( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int Toggle_Activate( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int Numeric_Changed( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_quit( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
#endif
