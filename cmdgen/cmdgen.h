/* cmdgen.h Definitions for Command Parser Generator
 * $Log$
 * Revision 1.6  2007/05/01 17:42:27  ntallen
 * Added %INTERFACE <name> spec
 *
 * Revision 1.5  2004/10/08 17:07:09  nort
 * Mostly keyword differences
 *
 * Revision 1.4  1994/09/08  13:45:54  nort
 * Removed new_memory() and free_memory() definitions now provided
 * by nortlib
 *
 * Revision 1.3  1992/10/27  08:38:20  nort
 * Added command line options
 *
 * Revision 1.2  1992/10/22  18:37:14  nort
 * Removed pipe_tail() prototype
 *
 * Revision 1.1  1992/10/20  19:55:06  nort
 * Initial revision
 *
 * Revision 1.1  1992/07/09  18:36:44  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include "config.h"

#if HAVE_STRCASECMP
  #if ! HAVE_STRICMP
    #define stricmp strcasecmp
  #endif
#endif
 
#define TABSIZE 4
struct vtyp {
  struct vtyp *next;
  char *type;
  char *member;
};

struct nt_t {
  struct nt_t *next; /* alpha ordering */
  char *name;
  struct vtyp *type;
  unsigned short number;
  struct {
	struct sub_t *first;
	struct sub_t *last;
  } rules;
};

struct sub_t {
  struct sub_t *next;
  struct nt_t *reduces;
  unsigned short rule_number;
  char *action;
  struct {
	struct sub_item_t *first;
	struct sub_item_t *last;
  } items;
};

struct sub_item_t {
  struct sub_item_t *next;
  unsigned char type;
  union {
	char *text;
	struct nt_t *nt;
	struct {
	  char *format;
	  char *symbol;
	  char *member;
	  char *prompt;
	} vspc;
  } u;
};
#define SI_WORD 1
#define SI_VSPC 2
#define SI_NT 3
#define SI_EOR 4

typedef union {
  char *str_val;
  struct nt_t *nt_val;
  struct sub_t *sub_val;
  struct sub_item_t *subi_val;
  struct vtyp *type_val;
} yystype;
#define YYSTYPE yystype

/* a list of rules in rule number, position order */
typedef struct rlst {
  struct rlst * next;
  unsigned short rule_number;
  unsigned short position;
  struct sub_item_t *si;
} rulelist;

typedef struct {
  unsigned char flag;
  unsigned short value;
} response;
#define RSP_SHIFT 1
#define RSP_REDUCE 2
#define RSP_SHIFT_REDUCE (RSP_SHIFT|RSP_REDUCE)
#define RSP_FULL_RED 4
/* If shifting, value is the next state. If reducing, value is
   the rule number by which we are reducing. */

/* list of terminals in alpha order */
typedef struct trmlst {
  struct trmlst *next;
  struct sub_item_t *term;
  response action;
} termlist;

/* list of non_terminals in numerical order */
typedef struct ntrmlst {
  struct ntrmlst *next;
  struct nt_t *nt;
  response action;
} ntermlist;

typedef struct stt {
  unsigned short state_number;
  unsigned char terminal_type;
  unsigned short n_terminals;
  rulelist *rules;
  response def_action;
  termlist *terminals;
  unsigned short term_offset; /* offset into trie */
  unsigned short prompt_offset; /* offset into prompts */
  ntermlist *non_terminals;
  unsigned short nonterm_offset; /* offset into non_terminals[] */
} state;

extern YYSTYPE yyval, yylval;
int yyparse(void); /* yyparse.y */
void app_error(unsigned int level, char *s, ...); /* yyparse.y */
int yylex(void); /* yylex.l */
extern int app_line; /* yyparse.y */
#define app_die(x) exit(x)
struct vtyp *get_vtype(char *type); /* vunion.c */
void get_vsymbol(struct sub_item_t *si, char *fmt, char *prompt); /* vunion.c */
void output_vdefs(void); /* vunion.c */
extern struct nt_t *non_terms; /* dstructs.c */
extern struct sub_t **rules; /* dstructs.c */
extern unsigned short n_rules; /* dstructs.c */
extern unsigned short n_nonterms; /* dstructs.c */
struct nt_t *non_terminal(char *name); /* dstructs.c */
void dmy_non_term(struct sub_t *sub); /* dstructs.c */
struct sub_t *new_sub(void); /* dstructs.c */
struct sub_item_t *new_sub_item(unsigned type); /* dstructs.c */
extern state **states; /* states.c */
extern unsigned short n_states; /* states.c */
extern unsigned short max_tokens; /* states.c */
state *new_state(void); /* states.c */
void add_rule_pos(state *state, unsigned short rnum, unsigned short pos); /* states.c */
void eval_states(void); /* states.c */
void print_state(FILE *sfile, state *st); /* cmdgen.c */
void print_rule_pos(FILE *fp, unsigned int rnum, int pos); /* cmdgen.c */
void output_trie(void); /* trie.c */
void output_prompts(void); /* prompts.c */
void output_shifts(void); /* states.c */
void output_states(void); /* states.c */
void output_rules(void); /* rules.c */
void new_interface( char *int_name );
void output_interfaces(void);

#define vfile ofile
#define efile stderr
