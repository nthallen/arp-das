/* param.h Parametrization of differences between text and photon */
#ifndef PARAM_H
#define PARAM_H

#include "dim.h"

extern int ColSpace;
extern dims_t CalcWordDims(char *text, int attr);
extern int DatumWidth(int ncols);
extern int DatumHeight(int nrows);
extern int RuleThickness;
#endif

