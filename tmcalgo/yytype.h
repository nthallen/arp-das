/* yytype.h
 * $Log$
 */
#ifndef _YYTYPE_H
#define _YYTYPE_H

struct cmddef {
  struct cmddef *next;
  long int cmdtime;
  char *cmdtext;
  int cmdflags;
};
/* cmdflags bits:
   1 CMDFLAGS_TMC: Set if command is a TMCSTAT, else COMMAND
*/
#define CMDFLAGS_TMC 1

struct cmdlst {
  struct cmddef *first;
  struct cmddef *last;
};

/* A state definition consists of a name and a list of timed commands */
struct stdef {
  char *name;
  struct cmddef *cmds;
};

struct prg {
  struct prg *next;
  int type;
  union {
	char *tmccmd;
	struct stdef *state;
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

#endif
