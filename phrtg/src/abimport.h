/* Import (extern) header for application - AppBuilder 2.03  */

#if defined(__cplusplus)
extern "C" {
#endif

#include "abdefine.h"

extern ApEventLink_t AbInternalLinks[];

extern ApWindowLink_t Console;
extern ApWindowLink_t Figure;
extern ApWidget_t AbWidgets[ 41 ];

extern ApMenuLink_t File_Menu;
extern ApMenuLink_t Variable_Menu;

#if defined(__cplusplus)
}
#endif


#ifdef __cplusplus
int phrtg_init( int argc, char **argv );
int menu_open_cmd( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_close_cmd( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_quit( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int console_destroyed( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int PanelSwitching( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_graph_curaxes( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_graph_overlay( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_graph_newpane( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_graph_newwin( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
int menu_file_report( PtWidget_t *widget, ApInfo_t *data, PtCallbackInfo_t *cbinfo );
#endif
