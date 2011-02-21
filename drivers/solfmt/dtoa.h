/* dtoa.h defines the dtoa structure
   Written March 23, 1987
*/
#define MAX_DTOAS 64
#define MAX_DTOA_SET_POINTS 64
#define MAX_SET_POINTS  (MAX_DTOAS * MAX_DTOA_SET_POINTS)

typedef struct {
  char *name;
  int n_set_points;
  int set_point_name[MAX_DTOA_SET_POINTS];
  int set_point_index[MAX_DTOA_SET_POINTS];
  int last_time;
  int last_state;
  int first_state;
} dtoa;

typedef struct {
  unsigned int address;
  unsigned int value;
} set_point;

extern set_point set_points[];
extern dtoa dtoas[];
extern int n_dtoas, n_set_points;
