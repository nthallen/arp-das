#ifndef MODPRINT_H_INCLUDED
#define MODPRINT_H_INCLUDED

#include "ptg_gen.h"
void FieldNo( PTG_OUTPUT_FILE f );
PTGNode print_vword( char *string, int row, int col, int attr );
PTGNode print_field_string( int row, int col, int width, int length );

#endif
