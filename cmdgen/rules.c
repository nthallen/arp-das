/* rules.c
 *
 * $Log$
 * Revision 1.7  1997/01/06 00:18:43  nort
 * Moved default case to top of rule_action() to eliminate warnings.
 *
 * Revision 1.6  1995/05/25  17:21:24  nort
 * Use standard nortlib compiler functions
 *
 * Revision 1.5  1994/02/14  21:23:12  nort
 * Older changes.
 * Added a redundant exit(1) after CMD_ERROR()
 *
 * Revision 1.4  1993/07/27  18:48:44  nort
 * Added default to rule_action(switch(){})
 *
 * Revision 1.3  1993/05/18  13:11:19  nort
 * Client/Server Support in output
 *
 * Revision 1.2  1992/10/27  01:09:39  nort
 * Added conditional around user actions
 *
 * Revision 1.1  1992/10/20  20:28:46  nort
 * Initial revision
 *
 * Revision 1.3  1992/09/03  16:06:44  nort
 * Moved vsp++ before the nterm_shift when generating rules.
 *
 * Revision 1.2  1992/07/10  19:31:47  nort
 * Resolved many stack inconsistencies.
 *
 * Revision 1.1  1992/07/09  18:36:44  nort
 * Initial revision
 *
  static cg_nonterm_type rule_action(unsigned short rule) {
	switch (rule) {
	  case 0:
	    Add comment showing rule
		<perform stated action>
		<fill in prev location>
		<vsp++ if a value was produced>
		return 1 if reducing rule 0, else 0
	  case 1:
	}
  }
 */
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "cmdgen.h"
#include "compiler.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

#define IND_NOT_SET 10000
#define ELT_HAS_VAL 1
#define ELT_IS_NT 2
#define MAXELTS 20
#define CASEINDENT 4
#define CONDINDENT (CASEINDENT+2)
#define BASEINDENT (CONDINDENT+2)
#define ACTINDENT (BASEINDENT+2)
#define indent(x) {int y=x;\
				   while (y>TABSIZE) {putc('\t',ofile);y-=TABSIZE;}\
				   while (y-->0) putc(' ',ofile);}

static void output_action(unsigned short rnum) {
  short ind = IND_NOT_SET, nsp;
  char *act;
  unsigned char eflags[MAXELTS], n_elts = 0;
  char *members[MAXELTS];
  struct sub_item_t *si;

  for (si = rules[rnum]->items.first; si != NULL; si = si->next) {
	if (n_elts >= MAXELTS)
	  compile_error(4, "Number of elements in rule %d exceeds arbitrary maximum",
			  rnum);
	switch (si->type) {
	  case SI_WORD:
		eflags[n_elts] = 0;
		break;
	  case SI_VSPC:
		eflags[n_elts] = ELT_HAS_VAL;
		members[n_elts] = si->u.vspc.member;
		break;
	  case SI_NT:
		eflags[n_elts] = ELT_IS_NT;
		if (si->u.nt->type != NULL) {
		  eflags[n_elts] |= ELT_HAS_VAL;
		  members[n_elts] = si->u.nt->type->member;
		}
		break;
	  default:
		compile_error(4, "Unexpected sub item type %d in output_action", si->type);
	}
	n_elts++;
  }
  eflags[n_elts] = 0;
  if (rules[rnum]->reduces != NULL
	  && rules[rnum]->reduces->type != NULL) {
	eflags[n_elts] = ELT_HAS_VAL;
	members[n_elts] = rules[rnum]->reduces->type->member;
  }

  act = rules[rnum]->action;
  if (act != NULL) {
	indent(CONDINDENT);
	fprintf(ofile, "#ifdef %s_ACTIONS\n",
	  rules[rnum]->reduces->name[0] == '&' ? "CLIENT" : "SERVER");
	indent(BASEINDENT);
	fprintf(ofile, "if (ioflags & IOF_EXECUTE)\n");
	indent(BASEINDENT);
	while (*act != '\0') switch(*act) {
	  case '\n':
		for (nsp = 0;; act++) {
		  if (*act == '\n') {
			putc(*act, ofile);
			nsp = 0;
		  } else if (*act == ' ') nsp++;
		  else if (*act == '\t') {
			nsp = (nsp + TABSIZE)/TABSIZE;
			nsp *= TABSIZE;
		  } else break;
		}
		if (ind == IND_NOT_SET) ind = nsp-ACTINDENT;
		nsp -= ind;
		if (nsp < BASEINDENT) { ind += nsp-BASEINDENT; nsp = BASEINDENT; }
		indent(nsp);
		break;
	  case '$':
		{
		  signed short eltnum, epos, dcnt, pcnt;
		  char *member;

		  /* These are defined in typedefs.skel:
			  define V(d,x) vstack[tstack[x-d].valpos]
			  define P(d,x) tstack[x-d].prev
			  define PP(d,x) (x-d)
		  */
		  ++act;
		  assert(*act != '\0');
		  if (isdigit(*act)) {
			for (eltnum = 0; isdigit(*act); act++)
			  eltnum = eltnum*10 + *act - '0';
		  } else {
			putc(*act++, ofile);
			break;
		  }
		  if (eltnum > n_elts) {
			compile_error(2, "$%d exceeds element count in rule %d:",
					eltnum, rnum);
			print_rule_pos(stderr, rnum, -1);
			fputc('\n', stderr);
			compile_error(3, "Cannot Continue");
		  }
		  if (eltnum == 0) epos = n_elts;
		  else epos = eltnum-1;
		  if ((eflags[epos] & ELT_HAS_VAL) == 0) {
			compile_error(2, "$%d has no value in rule %d",
					eltnum, rnum);
			print_rule_pos(stderr, rnum, -1);
			fputc('\n', stderr);
			compile_error(3, "Cannot Continue");
		  }
		  member = members[epos];
		  fprintf(ofile, "V(");
		  for (epos++, dcnt = 0, pcnt = 1; epos <= n_elts; epos++) {
			if (eflags[epos] & ELT_IS_NT) {
			  fprintf(ofile, "%d,P(", dcnt);
			  dcnt = 0;
			  pcnt++;
			} else dcnt++;
		  }
		  fprintf(ofile, "%d,tsp", dcnt);
		  while (pcnt-- > 0) putc(')', ofile);
		  fprintf(ofile, ".%s", member);
		}
		break;
	  default:
		putc(*act++, ofile);
		break;
	}
	putc('\n', ofile);
	if (rules[rnum]->reduces->name[0] != '&') {
	  indent(CONDINDENT);
	  fprintf(ofile, "#else\n");
	  indent(BASEINDENT);
	  fprintf(ofile, "saw_server_action = 1;\n");
	}
	indent(CONDINDENT);
	fprintf(ofile, "#endif\n");
  }

  /* <vsp++ if a value was produced> */
  if (eflags[n_elts] & ELT_HAS_VAL) {
	indent(CONDINDENT);
    fprintf(ofile, "vsp++;\n");
  }

  /* fill in prev location in tstack top. */
  indent(CONDINDENT);
  fprintf(ofile, "nterm_shift(%d, %d, PP(",
	rules[rnum]->reduces != NULL ? rules[rnum]->reduces->number : 0, rnum);
  { unsigned short epos, dcnt, pcnt;

	for (epos = 0, dcnt = 0, pcnt = 1; epos <= n_elts; epos++) {
	  if (eflags[epos] & ELT_IS_NT) {
		fprintf(ofile, "%d,P(", dcnt);
		dcnt = 0;
		pcnt++;
	  } else dcnt++;
	}
	fprintf(ofile, "%d,tsp", dcnt);
	while (pcnt-- > 0) putc(')', ofile);
  }
  fprintf(ofile, ");\n");

  indent(CONDINDENT);
  fprintf(ofile, "return(%d);\n", rules[rnum]->reduces != NULL ? 0 : 1);
}

void output_rules(void) {
  unsigned short rnum;
  
  fprintf(ofile, "static int rule_action(unsigned short rule) {\n");
  fprintf(ofile, "  switch (rule) {\n");
  indent(CASEINDENT);
  fprintf(ofile, "default:\n");
  indent(CONDINDENT);
  fprintf(ofile, "CMD_ERROR(\"Unexpected Rule in rule_action\");\n");
  for (rnum = 0; rnum < n_rules; rnum++) {
	indent(CASEINDENT);
	fprintf(ofile, "case %d:\n", rnum);
    indent(CONDINDENT);
	fprintf(ofile, "/*");
	if (rnum > 0)
	  fprintf(ofile, " &%s :", rules[rnum]->reduces->name);
	print_rule_pos(ofile, rnum, -1);
	fprintf(ofile, " */\n");
	output_action(rnum);
  }
  fprintf(ofile, "  }\n}\n");
}
