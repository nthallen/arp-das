/* vunion.c handles generation of the value stack union.

 * $Log$
 * Revision 1.4  1995/05/25  17:20:53  nort
 * Use standard nortlib compiler functions
 *
 * Revision 1.3  1993/02/11  15:12:44  nort
 * Skeleton Support
 *
 * Revision 1.2  1992/10/20  20:29:39  nort
 * Corrected IDs
 *
 * Revision 1.1  1992/10/20  20:28:46  nort
 * Initial revision
 *
 * Revision 1.2  1992/07/15  20:28:13  nort
 * Beta Release
 *
 * Revision 1.1  1992/07/09  18:36:44  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "cmdgen.h"
#include "compiler.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static struct vtyp *vtypes = NULL;
static unsigned n_members = 0;

struct vtyp *get_vtype(char *type) {
  struct vtyp *vt;
  char buf[10];
  
  for (vt = vtypes; vt != NULL; vt = vt->next)
	if (strcmp(vt->type, type) == 0) return(vt);
  vt = new_memory(sizeof(struct vtyp));
  vt->next = vtypes;
  vtypes = vt;
  vt->type = type;
  sprintf(buf, "vu%02d", n_members++);
  vt->member = strdup(buf);
  return(vt);
}

#define N_VARTYPES 5
static struct {
  char *symbol;
  char *type;
  struct vtyp *used;
} vartypes[N_VARTYPES] = {
  "VTP_SHRT", "short", NULL,
  "VTP_LONG", "long", NULL,
  "VTP_FLT", "float", NULL,
  "VTP_DBL", "double", NULL,
  "VTP_STR", "char *", NULL
};

/* varfmts. fmt is the format as the user specifies it.
   vtype is an offset into vartypes[] which helps define
   the union members required by these variables.
   used is flagged when any of these formats is used.
   The symbol will be pre-processor defined with the
   offset into the vardef[] array. Using those symbols,
   the vardef[] array definition can be placed entirely
   in the skeleton file.
*/
#define D_PROMPT "Enter Integer (Decimal: 123, Hex: 0x123F, Octal: 0123)"
#define X_PROMPT "Enter Hexadecimal Number"
#define O_PROMPT "Enter Octal Number"
#define F_PROMPT "Enter Floating Point Number"
#define W_PROMPT "Enter Word Terminated by <space>"
#define S_PROMPT "Enter Text Terminated by <CR>"

#define N_VARFMTS 12
static struct {
  char *fmt;
  unsigned char vtype;
  unsigned char used;
  char *symbol;
  char *def_prompt;
} varfmts[N_VARFMTS] = {
  "%d",  0, 0, "VAR_d", D_PROMPT,
  "%ld", 1, 0, "VAR_ld", D_PROMPT,
  "%x",  0, 0, "VAR_x", X_PROMPT,
  "%lx", 1, 0, "VAR_lx", X_PROMPT,
  "%o",  0, 0, "VAR_o", O_PROMPT,
  "%lo", 1, 0, "VAR_lo", O_PROMPT,
  "%f",  2, 0, "VAR_f", F_PROMPT,
  "%lf", 3, 0, "VAR_lf", F_PROMPT,
  "%w",  4, 0, "VAR_w", W_PROMPT,
  "%s",  4, 0, "VAR_s", S_PROMPT,
};

void get_vsymbol(struct sub_item_t *si, char *fmt, char *prompt) {
  int i, j;
  
  for (i = 0; i < N_VARFMTS; i++)
	if (stricmp(fmt, varfmts[i].fmt) == 0) break;
  if (i >= N_VARFMTS) compile_error(3, "Illegal Variable Format %s", fmt);
  else {
	varfmts[i].used = 1;
	j = varfmts[i].vtype;
	if (vartypes[j].used == NULL)
	  vartypes[j].used = get_vtype(vartypes[j].type);
	if (prompt == NULL) prompt = varfmts[i].def_prompt;
	si->u.vspc.format = fmt;
	si->u.vspc.symbol = varfmts[i].symbol;
	si->u.vspc.member = vartypes[j].used->member;
	si->u.vspc.prompt = prompt;
  }
}

/* output_vdefs() generates VAR_* and VTP_* defines as necessary */
void output_vdefs(void) {
  int i, j;
  
  /* Output VTP_ #defines */
  for (i = 0; i < N_VARTYPES; i++) {
	if (vartypes[i].used != NULL)
	  fprintf(ofile, "#define %s %s\n", vartypes[i].symbol,
					  vartypes[i].used->member);
  }
  
  /* Output VAR_ #defines */
  for (i = 0, j = 0; i < N_VARFMTS; i++) {
	if (varfmts[i].used)
	  fprintf(ofile, "#define %s %d\n", varfmts[i].symbol, j++);
  }
  
  /* Define value stack union vstack_type */
  if (vtypes != NULL) {
	struct vtyp *vt;
	
	fprintf(ofile, "typedef union {\n");
	for (vt = vtypes; vt != NULL; vt = vt->next)
	  fprintf(ofile, "  %s %s;\n", vt->type, vt->member);
	fprintf(ofile, "} vstack_type;\n");
  }
  Skel_copy(ofile, "vstack", vtypes != NULL);
}
