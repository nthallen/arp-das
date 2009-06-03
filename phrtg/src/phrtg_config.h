#ifndef PHRTG_CONFIG_H_
#define PHRTG_CONFIG_H_

#include <stdio.h>

enum Cfg_Keyword { Cfg_INVALID, Cfg_End, Cfg_Console_Pos, Cfg_MLF, Cfg_Detrend,
  Cfg_Figure, Cfg_Figure_Name, Cfg_Figure_Visible, Cfg_Figure_Area,
  Cfg_Pane, Cfg_Pane_Name, Cfg_Pane_Visible, Cfg_Pane_Color,
  Cfg_Axes, Cfg_Axes_Name, Cfg_Axis, Cfg_Graph, Cfg_Auto_Scale,
  Cfg_Limit_Min, Cfg_Limit_Max };

class RTG_Cfg_Ser {
  public:
    RTG_Cfg_Ser(const char *filename);
    ~RTG_Cfg_Ser();
    void keyword(Cfg_Keyword code);
    void varpath(RTG_Variable *var);
    void string(const char *str);
  private:
    FILE *fp;
    bool partial;
};

#endif /*PHRTG_CONFIG_H_*/
