/* callbacks.cc
 * Another location for static callback functions.
 */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

void Update_Text(int Name, char *text, Update_Source src ) {
  if (Name == ABN_Window_Name) {
    if (Current::Figure == NULL) {
      nl_error(2, "Update Window_Name with no Current::Window");
      return;
    }
    Current::Figure->rename(text);
  } else {
//    case ABN_Pane_Name:
//    case ABN_Axes_Name:
//    case ABN_Axis_Label:
//    case ABN_Line_Name:
//    default:
    nl_error(1, "Update_Text(%d,\"%s\") not implemented", Name, text);
  }
  if (src != from_widget)
    PtSetResource(AbGetABW(Name), Pt_ARG_TEXT_STRING, text, 0);
}

int Modify_Notify( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo ) {
  PtTextCallback_t *cb = (PtTextCallback_t *)cbinfo;
	/* eliminate 'unreferenced' warnings */
	apinfo = apinfo;
  Update_Text(ApName(widget), cb->text, from_widget);
	return( Pt_CONTINUE );
}

