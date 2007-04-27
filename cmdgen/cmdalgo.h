/* cmdalgo.h defines entry points into the cmdgen-based algorithms
 */
#ifndef CMDALGO_H_INCLUDED
#define CMDALGO_H_INCLUDED

void cmd_init(void);
void cmd_interact(void);
int cmd_batch(char *cmd, int test);
typedef struct {
  unsigned short state;
  unsigned short value;
} cmd_state;
void cmd_report(cmd_state *s);
int cmd_check(cmd_state *s);
void cis_initialize(void); /* in cmdgen.skel or .cmd */
void cis_terminate(void);  /* in cmdgen.skel of .cmd */
void cis_interfaces(void); /* generated */
#define CMDREP_QUIT 1000
#define CMDREP_SYNERR 2000
#define CMDREP_EXECERR 3000
#define CMDREP_TYPE(x) ((x)/1000)

struct ioattr_s;
#define IOFUNC_ATTR_T struct ioattr_s
extern IOFUNC_ATTR_T *cmdsrvr_setup_rdr( char *node );
extern void cmdsrvr_turf( IOFUNC_ATTR_T *handle, char *format, ... );
void ci_server(void); /* in tmlib/cis.c */

#endif
