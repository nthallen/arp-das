/* routines.h defines the data structures for routines.
   Written March 18, 1988
*/

typedef struct {
  char *name;
  long int fpos;
  int flin;
} rout;

#define MAX_ROUTINES 10
extern rout routines[];
extern int n_routines;
