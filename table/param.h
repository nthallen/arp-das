/* param.h Parametrization of differences between text and photon */
#ifndef PARAM_H
#define PARAM_H

#include "dim.h"
#include "ptg_gen.h"

extern int ColSpace;
extern dims_t CalcWordDims(char *text, int attr);
extern int DatumWidth(int ncols);
extern int DatumHeight(int nrows);
extern int RuleThickness;

void preview_window( PTG_OUTPUT_FILE f, const char *name, int w, int h );
void preview_label( PTG_OUTPUT_FILE f, const char *str, int r, int c );
void preview_field( PTG_OUTPUT_FILE f, int fldnum, int r, int c, int w, int h );
void preview_loop( PTG_OUTPUT_FILE f );
void preview_vrule( PTG_OUTPUT_FILE f, int r, int c, int h, int dbl );
void preview_hrule( PTG_OUTPUT_FILE f, int r, int c, int w, int dbl );
#endif

