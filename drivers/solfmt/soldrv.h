/* soldrv.h contains the definitions of the solenoid and set_point structures
   as used by the soldrv program (and distinct from those used by the solfmt
   program.
   Written April 9, 1987
   Modified April 24, 1990
*/

typedef struct {
  int open_cmd;
  int close_cmd;
  int status_addr;
  int status_mask;
} solenoid;

typedef struct {
  unsigned int address;
  unsigned int value;
} set_point;

extern int n_solenoids, n_set_points, n_modes, n_bytes, *mode_indices;
extern solenoid *solenoids;
extern set_point *set_points;
extern unsigned char *mode_code;
int read_sft(char *filename);

