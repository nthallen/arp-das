#include <stdio.h>
#include <stdlib.h>
#include "modprint.h"
#include "ptg_gen.h"

void FieldNo( PTG_OUTPUT_FILE f ) {
  static int fieldno = 1;

  PTG_OUTPUT_INT( f, fieldno++ );
}

PTGNode print_vword( char *string, int row, int col, int attr ) {
  PTGNode rv = PTGNULL;
  char *buf;

  while ( *string != '\0' ) {
	buf = malloc(2);
	buf[0] = *string++; buf[1] = '\0';
	rv = PTGSeq(rv,PTGString(row++,col,attr,PTGAsIs(buf)));
  }
  return rv;
}

PTGNode print_field_string( int row, int col, int width, int length ) {
  char *buf;
  int i;
  PTGNode rv;
  
  buf = malloc( width+1 );
  if ( buf == 0 ) exit(1);
  for ( i = 0; i < width; i++ ) buf[i] = ' ';
  buf[width] = '\0';
  rv = PTGNULL;
  for ( i = 0; i < length; i++ ) {
	rv = PTGSeq(rv, PTGString( row++, col, 2, PTGAsIs( buf ) ) );
  }
  /* don't free buf... */
  return rv;
}
