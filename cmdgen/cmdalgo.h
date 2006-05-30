/* cmdalgo.h defines entry points into the cmdgen-based algorithms
 * $Log$
 * Revision 1.6  1995/05/25  18:15:07  nort
 * Removed cmd_exit(): do it with atexit instead!
 *
 * Revision 1.5  1995/05/25  17:55:33  nort
 * Removed obsolete cmdalgo.c definitions
 * Added cmd_exit() prototype.
 *
 * Revision 1.4  1994/02/14  00:05:51  nort
 * Took out cis_initialize() and cis_terminate(): they
 * go better in nortlib.h with the other ci* prototypes.
 *
 * Revision 1.3  1994/02/13  23:22:02  nort
 * Latest cmdgen stuff
 *
 * Revision 1.2  1993/01/26  20:55:44  nort
 * Partial changes for new algorithms
 *
 * Revision 1.1  1993/01/09  15:51:16  nort
 * Initial revision
 *
 */
void cmd_init(void);
void cmd_interact(void);
int cmd_batch(char *cmd, int test);
typedef struct {
  unsigned short state;
  unsigned short value;
} cmd_state;
void cmd_report(cmd_state *s);
int cmd_check(cmd_state *s);
#define CMDREP_QUIT 1000
#define CMDREP_SYNERR 2000
#define CMDREP_EXECERR 3000
#define CMDREP_TYPE(x) ((x)/1000)

/* Message-level definition of command interpreter interface: */
#define CMDINTERP_NAME "cmdinterp"
#define CMD_INTERP_MAX 256
#define CMD_VERSION_MAX 80
#define CMD_PREFIX_MAX 9
typedef struct {
  unsigned char msg_type;
  char prefix[CMD_PREFIX_MAX];
  char command[CMD_INTERP_MAX];
} ci_msg;
typedef struct {
  unsigned char msg_type;
  char version[CMD_VERSION_MAX];
} ci_ver;
#define CMDINTERP_QUERY 255
#define CMDINTERP_SEND 254
#define CMDINTERP_TEST 253
#define CMDINTERP_QUIT 252
#define CMDINTERP_SEND_QUIET 251
