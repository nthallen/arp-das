/* yytype.h defines the union for YYSTYPE
   $Id$
   $Log$
   Revision 1.6  1995/11/15 04:19:38  nort
   Added cvtval member

 * Revision 1.5  1993/07/09  19:40:27  nort
 * *** empty log message ***
 *
 * Revision 1.4  1993/05/21  19:44:50  nort
 * Added State Variable Support
 *
 * Revision 1.3  1993/04/01  22:01:56  nort
 * Restructuring
 *
 * Revision 1.2  1992/07/21  20:37:19  nort
 * Alpha plus
 *
 * Revision 1.1  1992/07/20  15:30:58  nort
 * Initial revision
 *
*/
#ifndef _YYTYPE_H
#define _YYTYPE_H

#ifndef _TMCSTR_H
  #error Must include tmcstr.h before yytype.h
#endif
#ifndef _CALIBR_H
  #error Must include calibr.h before yytype.h
#endif

struct lexval {   /* lexical products */
  char *pretext;  /* text preceeding the token */
  char *toktext;  /* beginning of token in pretext */
  long int intval;
};

struct st_un {
  struct sttmnt stat;
  unsigned int type;
  unsigned int decl_type;
}; /* data for struct_union */

typedef union {
  struct lexval l; /* lexical products */
  struct sttmnt statval; /* provides just a sttmnt */
  struct typparts typeparts; /* provides a sttmnt and an integer */
  struct st_un struct_union; /* data for struct_union */
  struct declrtor *declval; /* for declarator definitions */
  struct {
    struct typparts typeparts;
    struct declrtor *first;
    struct declrtor *last;
  } decllist; /* for declaration definitions */
  rational ratval;
  struct deplst *depval; /* for lists of names with flags */
  struct nmlst *nlval; /* for lists of names */
  struct nm *nmval; /* for a reference */
  struct {
    struct tlstatpc *first;
    struct tlstatpc *last;
  } tlstatval;
  struct tmtype tmtval; /* for tmtype definitions */
  double doubval;
  struct pairlist plval;
  int intval;
  struct stateset *sttval;
  struct cvtfunc *cvtval;
} yystype;
#define YYSTYPE yystype

extern YYSTYPE yyval, yylval;
#endif
