/* solenoid.h defines the solenoid structure
 * $Log$
 * Revision 1.1  2011/02/21 18:26:05  ntallen
 * QNX4 version
 *
 * Revision 1.2  1993/09/28 17:08:03  nort
 * *** empty log message ***
 *
   Written March 23, 1987
*/
typedef struct {
  char *name;
  int open_cmd;
  int close_cmd;
  int status_bit;
  int last_time;
  int last_state;
  int first_state;
} solenoid;

#define MODE_SWITCH_OK (-2)
#define SOL_OPEN 0
#define SOL_CLOSE 1
#define MAX_SOLENOIDS 64
extern solenoid solenoids[];
extern int n_solenoids;
