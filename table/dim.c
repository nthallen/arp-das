#include <stdlib.h>
#include "dim.h"
#include "err.h"
#include "pdl_gen.h"
#include "dimfunc.h"

dim_t MKDIM( int Space, int Glue ) {
  dim_t dim;
  dim.Space = Space;
  dim.Glue = Glue;
  return dim;
}
dim_t MAXDIM( dim_t x, dim_t y) {
  dim_t maxdim;
  
  maxdim.Space = max( x.Space, y.Space );
  maxdim.Glue = max( x.Glue, y.Glue );
  return maxdim;
}
dim_t ADDDIM( dim_t x, dim_t y) {
  dim_t dimsum;
  
  dimsum.Space = x.Space + y.Space;
  dimsum.Glue = x.Glue + y.Glue;
  return dimsum;
}
dims_t MKDIMS( dim_t Width, dim_t Height ) {
  dims_t rv;
  rv.Width = Width; rv.Height = Height;
  return rv;
}

void NewInstance( DefTableKey Key, dim_t W, dim_t H,
				  int Row, int Col, int Real, int Offset ) {
  instance_t inst;
  
  inst = malloc(sizeof(struct instance_s));
  if ( inst == 0 )
	message(DEADLY, "No memory in NewInstance", 0, &curpos );
  inst->next = GetInstance(Key, NULL);
  inst->size.Width = W;
  inst->size.Height = H;
  inst->Row = Row;
  inst->Col = Col;
  inst->ForReal = Real;
  inst->Offset = Offset;
  ResetInstance( Key, inst );
}

instance_t PopInstance( DefTableKey Key ) {
  instance_t inst;
  
  inst = GetInstance(Key,NULL);
  if ( inst == 0 )
	message(DEADLY, "No Instance in PopInstance", 0, &curpos );
  ResetInstance(Key,inst->next);
  inst->next = NULL;
  return inst;
}

/* This is the Glue application stuff */
/* GlueDim determines the width of an inner component based on
   the specified width of the surrounding component.
   The Glue portion of Width is not used, and the Glue portion
   of the result is set to zero.
*/
dim_t GlueDim( dim_t MinDim, dim_t Dim ) {
  dim_t dim;
  
  if ( MinDim.Space > Dim.Space )
	message(DEADLY, "Inner dimension Exceeds outer", 0, &curpos );
  if ( MinDim.Glue > 0 ) {
	dim.Space = Dim.Space;
  } else dim.Space = MinDim.Space;
  dim.Glue = 0;
  return dim;
}

/* GlueSet performs the same operation as GlueDim, but returns
   information on how the glue in MinDim was used rather than
   the final dimension.
*/
glue_t GlueSet( dim_t MinDim, dim_t Dim ) {
  glue_t Glue;
  
  if ( MinDim.Space > Dim.Space )
	message(DEADLY, "Inner dimension Exceeds outer", 0, &curpos );
  if ( MinDim.Glue > 0 ) {
	Glue.Space = Dim.Space - MinDim.Space;
	Glue.Glue = MinDim.Glue;
  } else {
	Glue.Space = 0;
	Glue.Glue = 0;
  }
  return Glue;
}

static int GlueSpace( dim_t MinDim, glue_t Glue ) {
  if ( MinDim.Glue == 0 || Glue.Space == 0 ) return 0;
  if ( Glue.Glue <= 0 )
	message(DEADLY, "Zero Denominator in GlueApply", 0, &curpos );
  return (MinDim.Glue * Glue.Space + (Glue.Glue/2))/Glue.Glue;
}

/* GlueApply is used to calculate the dimension of one element of
   a list of elements when glue is involved. The Glue argument
   indicates how many glue spaces are available and over how many
   glue units they must be divided. Glue spaces are allocated to
   the current element in proportion to its Glue parameter.
*/
dim_t GlueApply( dim_t MinDim, glue_t Glue ) {
  dim_t dim;
 
  dim.Space = MinDim.Space + GlueSpace( MinDim, Glue );
  dim.Glue = 0;
  return dim;
}

/* GlueApplied performs almost the same calculation as GlueApply,
   but its purpose is to calculate how much glue space remains.
*/
glue_t GlueApplied( dim_t MinDim, glue_t Glue ) {
  glue_t remaining;
  
  remaining.Space = Glue.Space - GlueSpace( MinDim, Glue );
  remaining.Glue = Glue.Glue - MinDim.Glue;
  return remaining;
}

col_dim_t InitColSpecs( int n_cols ) {
  int i;
  col_dim_t ColSpec = malloc( sizeof(scol_dim_t) );

  if (ColSpec == 0)
	message(DEADLY, "ColSpec malloc failed", 0, &curpos);
  ColSpec->n_cols = n_cols;
  if ( n_cols > 0 ) {
	ColSpec->widths = malloc( n_cols * sizeof( dim_t ) );
	if ( ColSpec->widths == 0 )
	  message(DEADLY, "ColSpec->widths malloc failed", 0, &curpos);
	for ( i = 0; i < n_cols; i++ ) {
	  ColSpec->widths[i] = MKDIM0();
	}
  }
  return ColSpec;
}

/* Comparison of maxima is a multi-dimensional thing when the dim
   is multi-dimensional (glue...) */
void RecordColSpecs( col_dim_t ColSpec, int ColNum, dim_t width ) {
  if ( ColSpec->n_cols > ColNum )
	ColSpec->widths[ColNum] =
	  MAXDIM( ColSpec->widths[ColNum], width );
}

dim_t ColSpecWidth( col_dim_t ColSpec ) {
  dim_t sum = MKDIM0();
  int i;

  if ( ColSpec->n_cols == 0 ) return MKDIM0();
  for ( i = 0; i < ColSpec->n_cols; i++ )
	sum = ADDDIM( sum, ColSpec->widths[i] );
  sum.Space += ColSpec->n_cols - 1;
  return sum;
}

col_dim_t CalcColWidths( col_dim_t ColSpecs, dim_t MinWidth,
	dim_t Width ) {
  glue_t Glue;
  int i;
  col_dim_t ColWidths = InitColSpecs( ColSpecs->n_cols );

  Glue = GlueSet( MinWidth, Width );
  for ( i = 0; i < ColSpecs->n_cols; i++ ) {
	ColWidths->widths[i] = GlueApply( ColSpecs->widths[i], Glue );
	Glue = GlueApplied( ColSpecs->widths[i], Glue );
  }
  return ColWidths;
}

dim_t ColWidth( col_dim_t ColWidths, int ColNumber ) {
  if ( ColWidths == 0 || ColNumber >= ColWidths->n_cols )
	message( DEADLY, "Bad Input to ColWidth", 0, &curpos );
  return ColWidths->widths[ColNumber];
}
