/* yytype.h
 * $Log$
 * Revision 1.4  1999/12/03 16:46:11  nort
 * Added NoLog option to state definition
 *
 * Revision 1.3  1997/01/16 16:40:41  nort
 * comments
 *
 * Revision 1.2  1996/04/19  13:53:06  nort
 * R2 changes
 *
 * Revision 1.1  1993/05/18  20:37:23  nort
 * Initial revision
 *
 */
#ifndef _YYTYPE_H
#define _YYTYPE_H

struct substate {
  struct substate *next;
  char *name;
  int once;
  int state_case;
};

struct cmddef {
  struct cmddef *next;
  long int cmdtime;
  struct substate *substate;
  char *cmdtext;
  char *cmd2text;
  long int timeout;
  struct cmddef *else_stat;
  int cmdtype;
};
/* cmdtype values: */
#define CMDTYPE_TMC  1
#define CMDTYPE_QSTR 2
#define CMDTYPE_CMD  3
#define CMDTYPE_VAL  4
#define CMDTYPE_HOLD 5
#define CMDTYPE_VHOLD 6
/* for CMDTYPE_TMC, cmd2text is a tmc statement, and cmdtext
   is an optional list of dependencies.
   For CMDTYPE_QSTR cmdtext is the quoted string.
   For CMDTYPE_CMD cmdtext is the command.
   For CMDTYPE_VAL cmdtext is the variable to be validated.
   For CMDTYPE_HOLD cmdtext is the expression to hold for.
   For CMDTYPE_VHOLD cmdtext is the validatation list.
   Types _TMC, _HOLD and _VHOLD require substate generation.
*/

struct cmdlst {
  struct cmddef *first;
  struct cmddef *last;
};

/* A state definition consists of a name and a list of timed commands */
struct stdef {
  char *name;
  struct cmddef *cmds;
  char *filename;
  int nolog;
};

struct prtn {
  int partno;
  struct substate *idle_substate;
};

struct prg {
  struct prg *next;
  int type;
  union {
	struct cmddef *tmccmd;
	struct stdef *state;
	struct prtn *partition;
  } u;
};
#define PRGTYPE_STATE 1
#define PRGTYPE_TMCCMD 2
#define PRGTYPE_PARTITION 3

struct prglst {
  struct prg *first;
  struct prg *last;
};

typedef union {
  char *textval;
  long int intval;
  struct cmddef *cmdval;
  struct cmdlst cmdsval;
  struct stdef *stateval;
  struct prg prgval;
  struct prglst pglval;
} yystype;
#define YYSTYPE yystype

extern YYSTYPE yyval, yylval;
int yylex(void); /* yylex.l */
int yyparse(void); /* grammar.y */
extern int input_line_number; /* yylex.l */
char *new_state_name(char *); /* states.c */
void list_states(FILE *ofp); /* states.c */
void output_states(FILE *ofp); /* states.c */
extern struct prg *program; /* grammar.y */
void check_command(const char *command); /* commands.c */
void get_version(FILE *ofp); /* commands.c */
struct substate *new_substate( FILE *ofp, char *name, int once, int mkcase );
int get_state_case( char *statename, int slurp, int partno );
void output_mainloop( FILE *ofp ); /* states.c */

#endif
