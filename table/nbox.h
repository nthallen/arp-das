#ifndef NBOX_H_INCLUDED
#define NBOX_H_INCLUDED

#include "ptg_gen.h"

extern int IsVertical(int index);
void NewRule( int Row, int Col, int Width, int Height, int Attr, int index );
extern PTGNode print_rules( void );

#endif
