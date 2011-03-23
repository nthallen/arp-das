/* For inclusion by new cycle programs 3/13/97 */
#ifndef AVERAGE_H_INCLUDED
#define AVERAGE_H_INCLUDED

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct {
  int count;
  double sum;
} Average_Data;

extern double Average_Value( Average_Data *data );
extern void Average_Point( Average_Data *data, double pt );

#ifdef __cplusplus
};
#endif

#endif
