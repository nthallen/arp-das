/* solfmt.h
 * $Log$
 * Revision 1.2  2011/02/23 19:38:40  ntallen
 * Changes for DCCC
 *
 * Revision 1.1  2011/02/21 18:26:05  ntallen
 * QNX4 version
 *
 * Revision 1.3  2006/02/16 18:13:31  nort
 * Uncommitted changes
 *
 * Revision 1.2  1993/09/28  17:08:06  nort
 * *** empty log message ***
 *
 */

/*      compile.c       */
extern void describe(void);
extern void comp_waits(int j);
extern void compile(void);
extern void optimize(int mn);

/*      output.c        */
extern void output(char *ofile);
extern void read_status_addr(void);

/*      read_cmd.c      */
extern void read_cmd(void);
extern int cmd_set;

/*      read_d2a.c      */
extern void read_dtoa(void);

/*      read_mod.c      */
extern void init_modes(void);
extern void read_mode(void);

/*      read_sol.c      */
extern void read_sol(void);

/*      read_val.c      */
extern int get_change_code(int type, int dtoa_num);

/*      routines.c      */
extern void read_routine(void);

extern void read_proxy(void); /* read_pxy.c */
