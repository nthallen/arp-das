/** \file callbacks.cc
 * Another location for static callback functions.
 */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

void Update_Text(int Name, char *text, Update_Source src ) {
  if (Name == ABN_Figure_Name) {
    if (Current::Figure == NULL) {
      nl_error(2, "Update Figure_Name with no Current::Figure");
      return;
    }
    Current::Figure->rename(text, src);
  } else if (Name == ABN_Pane_Name) {
    if (Current::Pane == NULL) {
      nl_error(2, "Update Pane_Name with no Current::Pane");
      return;
    }
    Current::Pane->rename(text, src);
  } else if (Name == ABN_Axes_Name) {
    if (Current::Axes == NULL) {
      nl_error(2, "Update Axes_Name with no Current::Axes");
      return;
    }
    nl_error( -3, "Update(Axes_Name, \"%s\")", text );
    Current::Axes->rename(text, src);
  } else if (Name == ABN_Axis_Label) {
    if (Current::Axis == NULL) {
      nl_error(2, "Update Axis_Label with no Current::Axis");
      return;
    }
  } else if (Name == ABN_Line_Name) {
    if (Current::Line == NULL) {
      nl_error(2, "Update Line_Name with no Current::Line");
      return;
    }
    Current::Line->rename(text, src);
  } else {
    nl_error(1, "Update_Text(%d,\"%s\") not implemented", Name, text);
  }
  if (src != from_widget)
    PtSetResource(AbGetABW(Name), Pt_ARG_TEXT_STRING, text, 0);
}

int Modify_Notify( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo ) {
  PtTextCallback_t *cb = (PtTextCallback_t *)cbinfo->cbdata;
	/* eliminate 'unreferenced' warnings */
	apinfo = apinfo;
  Update_Text(ApName(widget), cb->text, from_widget);
	return( Pt_CONTINUE );
}

void Update_Color(int Name, PgColor_t rgb, Update_Source src ) {
  if (Name == ABN_Pane_Color) {
    if (Current::Pane == NULL)
      nl_error(2, "Update Pane_Color with no Current::Pane");
    else
      Current::Pane->set_bg_color(rgb, src);
  } else if (Name == ABN_Line_Color) {
    if (Current::Line == NULL)
      nl_error(2, "Update Line_Color with no Current::Line");
    else
      Current::Line->set_line_color(rgb, src);
  } else {
    nl_error(1,"Update_Color(%d) not implemented", Name);
  }
}

int Color_Changed( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo ) {
  PtColorSelCallback_t *cb = (PtColorSelCallback_t *)cbinfo->cbdata;
  /* eliminate 'unreferenced' warnings */
	apinfo = apinfo;
  Update_Color(ApName(widget), cb->rgb, from_widget);
	return( Pt_CONTINUE );
}

/**
 * This is only called from Update_Toggle() and so we can assume
 * the po if non-NULL is a Current object. I will forgo the check
 * that the tab is active, because it is complicated in the X/Y
 * cases and more or less irrelevant.
 */
static void po_set_visibility(plot_obj *po, const char *txt, long int value,
    int Name, Update_Source src) {
  if (po == NULL) {
    nl_error(2, "Update_Toggle(ABN_%s_Visible) with Current::%s NULL",
        txt, txt);
  } else {
    po->new_visibility = value;
    if (src != from_widget)
      PtSetResource(AbGetABW(Name), Pt_ARG_FLAGS,
          value ? Pt_TRUE : Pt_FALSE, Pt_SET);
  }
}

static void apply_limits() {
  /* This applies to Tab_X and Tab_Y and Current::Axis
   * It should only be called from_widget, though no harm
   * would come from calling it programmatically.
   * We need to retrieve values from ABW_Limit_Min and ABW_Limit_Max
   * and apply them to Current::Axis->limits.min & max
   */
  if ( Current::Axis == NULL ) {
    nl_error(2, "apply_limits() when Current::Axis == NULL");
    return;
  }
  if ( Current::Axes == NULL ) {
    nl_error(2, "apply_limits() when Current::Axes == NULL");
    return;
  }
  if ( Current::Axis->limits.limits_auto ) {
    nl_error(2, "apply_limits() while limits are auto");
    return;
  }
  double *minp, *maxp;
  PtGetResource(ABW_Limit_Min, Pt_ARG_NUMERIC_VALUE, &minp, 0);
  PtGetResource(ABW_Limit_Max, Pt_ARG_NUMERIC_VALUE, &maxp, 0);
  Current::Axis->set_scale(*minp, *maxp);
  Current::Axes->schedule_range_check();
  nl_error(-2, "Updated %s axis limits to (%.2f,%.2f)",
      Current::Axis->XY == Axis_X ? "X" : "Y",
      Current::Axis->limits.min, Current::Axis->limits.max);
}

static int Update_Derivation(int Name, long int value) {
  if ( Name != ABN_Detrend &&
       Name != ABN_Invert &&
       Name != ABN_PSD &&
       Name != ABN_Phase )
    return 0;
  if ( Current::Axes == NULL ) {
    nl_error(2, "Derivation toggle with no current axes");
    return 1;
  }
  if (Name == ABN_Detrend)
    Current::Axes->Detrend(value);
  else if ( Name == ABN_Invert)
    Current::Axes->Invert(value);
  else if ( Name == ABN_PSD )
    Current::Axes->PSD(value);
  else if ( Name == ABN_Phase )
    Current::Axes->Phase(value);
  Current::Axes->Update_Axis_Pane();
  return 1;
}

void Update_Toggle(int Name, long int value, Update_Source src ) {
  if ( Name == ABN_Figure_Visible )
    po_set_visibility(Current::Figure, "Figure", value, Name, src);
  else if (Name == ABN_Pane_Visible)
    po_set_visibility(Current::Pane, "Pane", value, Name, src);
  else if (Name == ABN_Axes_Visible)
    po_set_visibility(Current::Axes, "Axes", value, Name, src);
  else if (Name == ABN_Line_Visible)
    po_set_visibility(Current::Line, "Line", value, Name, src);
  else if (Name == ABN_Apply_Limits) {
    apply_limits();
  } else if (Name == ABN_Auto_Scale) {
    /* This is on Tab_X and Tab_Y
     * It refers to Current::Axis (and if selected, overlaid axises)
     * We need to toggle Pt_GHOST and Pt_BLOCKED on ABW_Limits_Group
     * and update Current::Axis->limits.limits_auto.
     * Turning auto off should not require any updates, but
     * turning it on will.
     */
    if (Current::Axis == NULL || Current::Axes == NULL) {
      nl_error(2,"Toggle Auto_Scale with no Current::Axis or Axes");
    } else {
      Current::Axis->limits.limits_auto = value;
      if ( value ) {
        // Current::Axis->axis_range_updated = true;
        Current::Axes->schedule_range_check();
        Current::Axis->range.clear();
      }
      if ((Current::Axis->XY == Axis_X && Current::Tab == Tab_X)
        || (Current::Axis->XY == Axis_Y && Current::Tab == Tab_Y)) {
        long is_auto = value ? Pt_TRUE : Pt_FALSE;
        PtSetResource(ABW_Limit_Min, Pt_ARG_FLAGS, is_auto,
            Pt_GHOST|Pt_BLOCKED );
        PtSetResource(ABW_Limit_Max, Pt_ARG_FLAGS, is_auto,
            Pt_GHOST|Pt_BLOCKED );
        PtSetResource(ABW_Apply_Limits, Pt_ARG_FLAGS, is_auto,
            Pt_GHOST|Pt_BLOCKED );
        if (src != from_widget)
          PtSetResource(ABW_Auto_Scale, Pt_ARG_FLAGS, is_auto, Pt_SET);
      }
    }
  } else if (!Update_Derivation(Name, value) )
    nl_error( 1, "Update_Toggle(%d) not implemented", Name);
}

int Toggle_Activate( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo ) {
  PtBasicCallback_t *cb = (PtBasicCallback_t*)cbinfo->cbdata;
  /* eliminate 'unreferenced' warnings */
	apinfo = apinfo;
	Update_Toggle(ApName(widget), cb->value, from_widget);
  plot_obj::render_one();
	return( Pt_CONTINUE );
}

void Update_Numeric(int Name, double value, Update_Source src ) {
  if (Name == ABN_Limit_Min) {
    if (Current::Axis == NULL) return;
    if (src != from_widget &&
        (Current::Tab == Tab_X || Current::Tab == Tab_Y))
      PtSetResource(ABW_Limit_Min, Pt_ARG_NUMERIC_VALUE, &value, 0);
    apply_limits();
  } else if (Name == ABN_Limit_Max) {
    if (Current::Axis == NULL) return;
    if (src != from_widget &&
        (Current::Tab == Tab_X || Current::Tab == Tab_Y))
      PtSetResource(ABW_Limit_Max, Pt_ARG_NUMERIC_VALUE, &value, 0);
    apply_limits();
  } else if (Name == ABN_Line_Column) {
    if (value < 0) return;
    unsigned col = (int)value;
    if ( col >= Current::Graph->lines.size())
      return;
    nl_error(-2, "Switching to column %u", col);
    Current::Graph->lines[col]->got_focus(focus_from_user);
  } else nl_error(1,"Update_Numeric(%s) not implemented", Name);
}

/* Current only for NumericInteger Widgets */
int Numeric_Changed( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo ) {
  PtNumericIntegerCallback_t *cb = (PtNumericIntegerCallback_t *)cbinfo->cbdata;
  /* eliminate 'unreferenced' warnings */
	apinfo = apinfo;
	Update_Numeric(ApName(widget), cb->numeric_value, from_widget);
  plot_obj::render_one();
	return( Pt_CONTINUE );
}

