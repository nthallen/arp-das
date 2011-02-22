/* get_token.c is yet another such program.  This one is for the solenoid
   format translator and is fairly simple, along the lines of the original
   TM get_token.
   $Log$
   Revision 1.1  2011/02/21 18:26:05  ntallen
   QNX4 version

   Revision 1.4  2006/02/16 18:13:22  nort
   Uncommitted changes

 * Revision 1.3  1993/09/28  17:14:32  nort
 * *** empty log message ***
 *
 * Revision 1.2  1993/02/17  14:51:33  nort
 * replaced MAX_INPUT (conflict with limits.h) with MAX_INPUT_CHARS
 *
 * Revision 1.1  1992/09/21  18:21:44  nort
 * Initial revision
 *
   Written March 23, 1987
   Modified July 1991 for QNX.
   Modified 4/17/92 for QNX 4.
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "tokens.h"
#include "solenoid.h"
#include "routines.h"
#include "dtoa.h"
#include "solfmt.h"
#include "proxies.h"
#include "nortlib.h" /* for nl_error */

static char rcsid[] =
      "$Id$";

#define iscsym(c) ((isalnum((c))||((((c))&0xFF)==0x5F)))

#define MAX_INPUT_CHARS 80
char gt_input[MAX_INPUT_CHARS+1];
int gt_number;

static FILE *gt_fp = NULL;
static char gt_filename[40];
static int gt_line_no;

int open_token_file(char *name) {
  if (gt_fp != NULL) nl_error(3,"Attempt to open second file\n");
  gt_fp = fopen(name, "r");
  if (gt_fp == NULL) return(1);
  strcpy(gt_filename, name);
  gt_line_no = 1;
  return(0);
}

struct {
  char *text;
  int token;
} reserves[] = {
  { "solenoid", TK_SOLENOID },
  { "open", TK_OPEN },
  { "close", TK_CLOSE },
  { "resolution", TK_RESOLUTION },
  { "mode", TK_MODE },
  { "initialize", TK_INITIALIZE },
  { "routine", TK_ROUTINE },
  { "select", TK_SELECT },
  { "status_bytes", TK_STATUS_BYTES },
  { "DtoA", TK_DTOA },
  { "Proxy", TK_PROXY },
  { "Command_Set", TK_CMD_SET },
  { NULL, 0 }
};

int get_token(void) {
  int c, i;

  if (gt_fp == NULL) return(TK_EOF);
  for (;;) {
    c = gt_getc();
    switch (c) {
      case ' ':
      case '\t':
      case '\n':
      case '\f':
        continue;
      case '=': return(TK_EQUAL);
      case ':': return(TK_COLON);
      case '{': return(TK_LBRACE);
      case '}': return(TK_RBRACE);
      case '/': return(TK_SLASH);
      case ';':
        for (;;) {
          c = gt_getc();
          if (c == '\n' || c == EOF) break;
        }
        continue;
      case '\'':
        c = gt_getc();
        if (c == '\\') c = gt_getc();
        gt_number = c;
        if (gt_getc() != '\'')
          filerr("Character Constant Requires Two Apostrophes");
        return(TK_CHAR_CONSTANT);
      case EOF:
        fclose(gt_fp);
        gt_fp = NULL;
        return(TK_EOF);
      case '0':
        c = gt_getc();
        if (c == 'x' || c == 'X') {
          if (fscanf(gt_fp, "%x", &gt_number) != 1)
            filerr("Hex Number Syntax Error\n");
        } else {
          gt_ungetc(c);
          if (isdigit(c)) {
            if (fscanf(gt_fp, "%o", &gt_number) != 1)
              filerr("Octal Number Syntax Error\n");
          } else gt_number = 0;
        }
        return(TK_NUMBER);
      default:
        if (isdigit(c)) {
          gt_ungetc(c);
          if (fscanf(gt_fp, "%d", &gt_number) != 1)
            filerr("Decimal Number Syntax Error\n");
          return(TK_NUMBER);
        } else if (iscsym(c)) {
          for (i = 0; i < MAX_INPUT_CHARS;) {
            gt_input[i++] = c;
            c = gt_getc();
            if (!iscsym(c)) break;
          }
          gt_ungetc(c);
          gt_input[i] = '\0';
          /* search reserve word list */
          for (i = 0; reserves[i].text != NULL; i++)
            if (stricmp(gt_input, reserves[i].text) == 0)
              return(reserves[i].token);
          /* search solenoid name list */
          for (i = 0; i < n_solenoids; i++)
            if (stricmp(gt_input, solenoids[i].name) == 0) {
              gt_number = i;
              return(TK_SOLENOID_NAME);
            }
          for (i = 0; i < n_routines; i++)
            if (stricmp(gt_input, routines[i].name) == 0) {
              gt_number = i;
              return(TK_ROUTINE_NAME);
            }
          for (i = 0; i < n_dtoas; i++)
            if (stricmp(gt_input, dtoas[i].name) == 0) {
              gt_number = i;
              return(TK_DTOA_NAME);
            }
		  for (i = 0; i < n_proxies; i++)
			if (stricmp(gt_input, proxies[i].name) == 0) {
			  gt_number = i;
			  return(TK_PROXY_NAME);
			}
          return(TK_UNDEFINED_STRING);
        } else filerr("Unknown Character");
    }
  }
}

int gt_getc(void) {
  int c;

  if (gt_fp == NULL) return(EOF);
  c = getc(gt_fp);
  if (c == '\n') gt_line_no++;
  return(c);
}

void gt_ungetc(int c) {
  if (c == '\n') gt_line_no--;
  ungetc(c, gt_fp);
}

long int gt_fpos(int *line) {
  extern long int ftell(FILE *);

  *line = gt_line_no;
  return(ftell(gt_fp));
}

int gt_spos(long int pos, int line) {
  gt_line_no = line;
  return(fseek(gt_fp, pos, 0));
}

void filerr(char * cntrl,...) {
  va_list ap;
  char buf[128];

  if (gt_fp == NULL) sprintf(buf, "Solfmt - ");
  else sprintf(buf, "%s:%d - ", gt_filename, gt_line_no);
  va_start(ap, cntrl);
  vsprintf(&buf[strlen(buf)], cntrl, ap);
  va_end(ap);
  nl_error(3,buf);
}
