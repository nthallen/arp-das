#include <stdlib.h>
#include "csm.h"
#include "err.h"
#include "nbox.h"
#include "modprint.h"
#include "PtgCommon.h"
#include "ptg_gen.h"

typedef struct tblrule {
  struct tblrule *next;
  int Row, Col, Width, Height;
  int Vertical, Lines;
  int Attr;
  int Dims[2][2]; /* [vert][edge] */
  int ext[2][2]; /* [vert][edge] */
  int plus[2]; /* [edge] */
  int t;
} *TableRule;

#define OPP(x) (1-(x))

static TableRule TableRules;

/* index is the StringTable index of the rule string */
int IsVertical(int index) {
  char *rule = StringTable(index);
  if ( *rule == '+' ) rule++;
  switch ( *rule ) {
	case '-':
	case '=':
	  return 0;
	case '|':
	  return 1;
	default:
	  message(DEADLY, "Bad character in rule string", 0, &curpos );
  }
}

void NewRule( int Row, int Col, int Width, int Height,
				int Attr, int index ) {
  TableRule new;
  unsigned char *rulecode;
  int Vertical, Lines;
  int preplus = 0, postplus = 0;

  rulecode = StringTable(index);
  if ( *rulecode == '+' ) {
	preplus = 1;
	rulecode++;
  }
  switch (*rulecode) {
	case '-':
	  Vertical = 0; Lines = 1; break;
	case '=':
	  Vertical = 0; Lines = 2; break;
	case '|':
	  Vertical = 1;
	  if ( rulecode[1] == '|' ) {
		rulecode++;
		Lines = 2;
	  } else Lines = 1;
	  break;
	default:
	  message( DEADLY, "Unknown code in New Rule", 0, &curpos );
  }
  if ( rulecode[1] == '+' ) postplus = 1;

  new = malloc(sizeof(struct tblrule));
  if ( new == 0 )
	message(DEADLY, "Out of memory in NewRule", 0, &curpos );

  new->Row = new->Dims[1][0] = Row;
  new->Col = new->Dims[0][0] = Col;
  new->Width = Width;
  new->Dims[0][1] = Col+Width;
  new->Height = Height;
  new->Dims[1][1] = Row+Height;
  new->Vertical = Vertical;
  new->Lines = Lines;
  new->plus[0] = preplus;
  new->plus[1] = postplus;
  new->ext[0][0] = new->ext[0][1] =
	 new->ext[1][0] = new->ext[1][1] = 0;
  new->Attr = Attr;

  switch ( Lines ) {
    case 1: new->t = 1; break;
    case 2: new->t = 3; break;
    default: message(DEADLY,"Lines not 1 or 2", 0, &curpos );
  }
  { int thick;
    thick = new->Vertical ? Width : Height;
    new->ext[OPP(new->Vertical)][0] = -(thick - new->t)/2;
    new->ext[OPP(new->Vertical)][1] = -(thick - new->t + 1)/2;
  }

  new->next = TableRules;
  TableRules = new;
}

static void connect_rules( void ) {
  TableRule R, S;
  int edge, sedge;

  for ( R = TableRules; R != 0; R = R->next ) {
	if ( R->plus[0] || R->plus[1] ) {
	  int RV = R->Vertical;
	  for ( S = TableRules; S != 0; S = S->next ) {
	    int SV = S->Vertical;
		if ( SV != RV ) {
	    int SV = S->Vertical;
		  for ( edge = 0; edge <= 1; edge++ ) {
		    /* Check for plus extension */
		    if ( R->plus[edge] &&
		         S->Dims[RV][OPP(edge)] == R->Dims[RV][edge] &&
		         S->Dims[SV][0] <= R->Dims[SV][0] &&
		         S->Dims[SV][1] >= R->Dims[SV][1] ) {
		      R->ext[RV][edge] = - S->ext[RV][OPP(edge)];
			  /* Check for non-plus shortening */
			  for ( sedge = 0; sedge <= 1; sedge++ ) {
			    if ( ! S->plus[sedge] &&
			         S->Dims[SV][sedge] ==
			           R->Dims[SV][sedge] ) {
			      int ext = R->ext[SV][sedge];
			      if ( S->ext[SV][sedge] == 0 ||
			           S->ext[SV][sedge] < ext )
			        S->ext[SV][sedge] = ext;
			    }
			  }
		    }
		  }
		}
	  }
	}
  }
}

PTGNode print_rules( void ) {
  TableRule Rule;
  PTGNode rv = PTGNULL, nptg;

  connect_rules();
  for ( Rule = TableRules; Rule != 0; Rule = Rule->next ) {
    int x, y, l;
    int v = Rule->Vertical;
    int dbl = (Rule->Lines == 2);
    x = Rule->Dims[0][0] - Rule->ext[0][0];
    y = Rule->Dims[1][0] - Rule->ext[1][0];
    l = Rule->Dims[v][1] + Rule->ext[v][1] - Rule->Dims[v][0] + Rule->ext[v][0];

	nptg = Rule->Vertical ?
	  PTGVRule( y, x+dbl, l, dbl ) :
	  PTGHRule( y+dbl, x, l, dbl );
	rv = PTGSeq(rv, nptg);
  }
  return rv;
}
