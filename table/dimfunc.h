#ifndef DIMFUNC_H_INCLUDED
#define DIMFUNC_H_INCLUDED

extern void NewInstance( DefTableKey Key,
  dim_t W, dim_t H, int Row, int Col, int Real, int Offset );
extern instance_t PopInstance( DefTableKey Key );

#endif
