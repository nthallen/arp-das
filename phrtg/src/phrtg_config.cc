/** \file phrtg_config.cc
 * Configuration Support
 */

/* Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Local headers */
#include "ablibs.h"
#include "phrtg.h"
#include "phrtg_config.h"
#include "abimport.h"
#include "proto.h"

#include "msg.h"
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include "nl_assert.h"

int RTG_Cfg::Serialize( PtWidget_t *widget, ApInfo_t *apinfo,
    PtCallbackInfo_t *cbinfo ) {
  const char *filename = ".phrtgrc";
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  RTG_Cfg_Ser ser(filename);
  ser.keyword(Cfg_End);
  ser.string("File");
  return Pt_CONTINUE;
}

RTG_Cfg_Ser::RTG_Cfg_Ser(const char *filename) {
  partial = false;
  fp = fopen(filename, "w");
  if (fp == NULL) {
    ApError( ABW_Console, errno, "PhRTG", "Unable to open configuration file",
               filename );
  }
}

RTG_Cfg_Ser::~RTG_Cfg_Ser() {
  if (fp != NULL) {
    if (partial) {
      fprintf(fp, "\n");
    }
    fclose(fp);
    fp = NULL;
  }
}

void RTG_Cfg_Ser::keyword(Cfg_Keyword code) {
  const char *kwd;
  if ( fp == NULL ) return;
  switch (code) {
    case Cfg_End: kwd = "End"; break;
    case Cfg_Console_Pos: kwd = "Console_Pos"; break;
    case Cfg_MLF: kwd = "MLF"; break;
    case Cfg_Detrend: kwd = "Detrend"; break;
    case Cfg_Figure: kwd = "Figure"; break;
    case Cfg_Figure_Name: kwd = "Figure_Name"; break;
    case Cfg_Figure_Visible: kwd = "Figure_Visible"; break;
    case Cfg_Figure_Area: kwd = "Figure_Area"; break;
    case Cfg_Pane: kwd = "Pane"; break;
    case Cfg_Pane_Name: kwd = "Pane_Name"; break;
    case Cfg_Pane_Visible: kwd = "Pane_Visible"; break;
    case Cfg_Pane_Color: kwd = "Pane_Color"; break;
    case Cfg_Axes: kwd = "Axes"; break;
    case Cfg_Axes_Name: kwd = "Axes_Name"; break;
    case Cfg_Axis: kwd = "Axis"; break;
    case Cfg_Graph: kwd = "Graph"; break;
    case Cfg_Auto_Scale: kwd = "Auto_Scale"; break;
    case Cfg_Limit_Min: kwd = "Limit_Min"; break;
    case Cfg_Limit_Max: kwd = "Limit_Max"; break;
    default:
      ApError( ABW_Console, errno, "PhRTG", "Undefined Keyword",
                 "RTG_Cfg_Ser::keyword" );
      kwd = "UNDEFINED"; break;
  }
  if (partial) fprintf(fp, "\n");
  fprintf(fp, "%s", kwd);
  partial = true;
}

void RTG_Cfg_Ser::string(const char *str) {
  if (fp == NULL) return;
  fprintf(fp, " %s", str);
  partial = true;
}
