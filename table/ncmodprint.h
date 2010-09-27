#ifndef MODPRINT_H_INCLUDED
#define MODPRINT_H_INCLUDED

#include "ptg_gen.h"
PTGNode print_vword( char *string, int tblname, int row, int col, int attr );
PTGNode print_field_string( int tblname, int row, int col, int width, int length );

#endif
