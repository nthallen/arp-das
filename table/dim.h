/* dim.h */
#ifndef DIM_H_INCLUDED
#define DIM_H_INCLUDED

typedef struct {
  int Space;
  int Glue;
} dim_t;

typedef struct {
  dim_t Width;
  dim_t Height;
} dims_t;

extern dim_t MKDIM( int Space, int Glue );
extern dim_t MAXDIM( dim_t x, dim_t y);
extern dim_t ADDDIM( dim_t x, dim_t y);
#define SPACE(x) ((x).Space)
#define GLUE(x) ((x).Glue)
#define SPACEGLUE(x) SPACE(x), GLUE(x)
#define MKDIM0() MKDIM(0,0)
extern dims_t MKDIMS( dim_t w, dim_t h );
#define WIDTH(x) ((x).Width)
#define HEIGHT(x) ((x).Height)

typedef struct instance_s {
  struct instance_s *next;
  dims_t size;
  int Row;
  int Col;
  int ForReal;
  int Offset;
} *instance_t;

#define INST_SIZE(x) ((x)->size)
#define INST_ROW(x) ((x)->Row)
#define INST_COL(x) ((x)->Col)
#define INST_EXISTS(x) ((x)->ForReal)
#define INST_OFFSET(x) ((x)->Offset)

#define DEF_DATUM_WID 5
#define DEF_DATUM_HT 1

typedef dim_t glue_t;
extern dim_t GlueDim( dim_t MinWidth, dim_t Width );
extern glue_t GlueSet( dim_t MinWidth, dim_t Width );
extern dim_t GlueApply( dim_t MinWidth, glue_t Glue );
extern glue_t GlueApplied( dim_t MinWidth, glue_t Glue );

typedef struct {
  int n_cols;
  dim_t *widths;
} scol_dim_t, *col_dim_t;

extern col_dim_t InitColSpecs( int n_cols );
extern dim_t ColSpecWidth( col_dim_t ColSpec );
extern void RecordColSpecs( col_dim_t ColSpec, int ColNum, dim_t width );
extern col_dim_t CalcColWidths( col_dim_t ColSpecs, dim_t MW, dim_t W );
extern dim_t ColWidth( col_dim_t ColWidths, int ColNumber );
#endif
