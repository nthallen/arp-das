/* dtoe.h defines how to output floats in E format */

char *dtoe( double v, int width, char *obuf );
#define edisplay(d,r,c,w,v) {\
  static double sv_ = -1;\
  if (sv_ != v) { sv_ = v; cdisplay(d,r,c,dtoe(v,w,0)); }\
}
