/* modes.h defines structures needed for storing modes.
   Written March 24, 1987
*/
typedef struct chng {
  int type;
  int t_index;
  int state;
  int time;
  struct chng *next;
} change;

typedef struct {
  change *init;
  change *first;
  int length;
  int res_num;
  int res_den;
  int index;
  unsigned int count;
  int iters;
  int next_mode;
} mode;

#define MAX_MODES 256
#define MAX_MULT_STRINGS 256
#define DCCC_MAX_CMD_BUF 250
extern mode modes[];
extern char mode_code[];
extern int mci;
extern char *dccc_strs[MAX_MULT_STRINGS];
extern int n_dccc_strs;

void add_change(change *nc, change **base); /* read_mod.c */
