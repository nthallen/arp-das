#include <stdlib.h>
#include "csm.h"
#include "err.h"
#include "nbox.h"
#include "modprint.h"

#define RL_BT 1
#define RL_TP 3
#define RL_RT  9
#define RL_LT 27

typedef struct tblrule {
  struct tblrule *next;
  int Row, Col, Width, Height;
  int Vertical, Lines, preplus, postplus;
  int Attr;
  unsigned char *rule;
} *TableRule;

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
  unsigned char *rule, *rulecode;
  unsigned char middle;
  int Length, Vertical, Lines;
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

  Length = Width * Height;
  new = malloc(sizeof(struct tblrule));
  rule = malloc( Length+1 );
  if ( new == 0 || rule == 0 )
	message(DEADLY, "Out of memory in NewRule", 0, &curpos );
  new->Row = Row;
  new->Col = Col;
  new->Width = Width;
  new->Height = Height;
  new->Vertical = Vertical;
  new->Lines = Lines;
  new->preplus = preplus;
  new->postplus = postplus;
  new->Attr = Attr;

  middle = ( Vertical ? RL_TP+RL_BT : RL_LT+RL_RT ) * Lines;
  if (Length == 1) {
	if (preplus) {
	  if (postplus) rule[0] = middle;
	  else rule[0] = (Vertical ? RL_TP : RL_LT)*Lines;
	} else if ( postplus ) {
	  rule[0] = (Vertical ? RL_BT : RL_RT)*Lines;
	} else rule[0] = 0;
  } else if (Length > 0) {
	rule[0] = preplus ? middle : ((Vertical ? RL_BT : RL_RT)*Lines);
	{ int i;
	  for ( i = 1; i < Length-1; i++ ) rule[i] = middle;
	}
	rule[Length-1] = postplus ? middle : ((Vertical ? RL_TP : RL_LT)*Lines);
  }
  rule[Length] = '\0';
  new->rule = rule;
  new->next = TableRules;
  TableRules = new;
}

static void add_bits( TableRule S, int row, int col, int t, int lines ) {
  int c, i;
  if ( row >= S->Row && row < S->Row + S->Height &&
	   col >= S->Col && col < S->Col + S->Width ) {
	i = row-S->Row+col-S->Col;
	c = (S->rule[i]/t)%3;
	if ( c < lines )
	  S->rule[i] += t * (lines-c);
  }
}

static void connect_rules( void ) {
  TableRule R, S;
  unsigned char c, cc;
  
  for ( R = TableRules; R != 0; R = R->next ) {
	for ( S = TableRules; S != 0; S = S->next ) {
	  if ( R->Vertical ) {
		if ( R->preplus )
		  add_bits( S, R->Row-1, R->Col, RL_BT, R->Lines );
		if ( R->postplus )
		  add_bits( S, R->Row+R->Height, R->Col, RL_TP, R->Lines );
	  } else {
		if ( R->preplus )
		  add_bits( S, R->Row, R->Col-1, RL_RT, R->Lines );
		if ( R->postplus )
		  add_bits( S, R->Row, R->Col+R->Width, RL_LT, R->Lines );
	  }
	}
  }
}

unsigned char scrchar[] = {
  '\x20', /*  0 = 0000 */
  '\xB3', /*  1 = 0001 */
  '\xBA', /*  2 = 0002 */
  '\xB3', /*  3 = 0010 */
  '\xB3', /*  4 = 0011 */
  '\xBA', /*  5 = 0012 */
  '\xBA', /*  6 = 0020 */
  '\xBA', /*  7 = 0021 */
  '\xBA', /*  8 = 0022 */
  '\xC4', /*  9 = 0100 */
  '\xDA', /* 10 = 0101 */
  '\xD6', /* 11 = 0102 */
  '\xC0', /* 12 = 0110 */
  '\xC3', /* 13 = 0111 */
  '\xD6', /* 14 = 0112 */
  '\xD3', /* 15 = 0120 */
  '\xD3', /* 16 = 0121 */
  '\xC7', /* 17 = 0122 */
  '\xCD', /* 18 = 0200 */
  '\xD5', /* 19 = 0201 */
  '\xC9', /* 20 = 0202 */
  '\xD4', /* 21 = 0210 */
  '\xC6', /* 22 = 0211 */
  '\xC9', /* 23 = 0212 */
  '\xC8', /* 24 = 0220 */
  '\xC8', /* 25 = 0221 */
  '\xCC', /* 26 = 0222 */
  '\xC4', /* 27 = 1000 */
  '\xBF', /* 28 = 1001 */
  '\xB7', /* 29 = 1002 */
  '\xD9', /* 30 = 1010 */
  '\xB4', /* 31 = 1011 */
  '\xB7', /* 32 = 1012 */
  '\xBD', /* 33 = 1020 */
  '\xBD', /* 34 = 1021 */
  '\xB6', /* 35 = 1022 */
  '\xC4', /* 36 = 1100 */
  '\xC2', /* 37 = 1101 */
  '\xD2', /* 38 = 1102 */
  '\xC1', /* 39 = 1110 */
  '\xC5', /* 40 = 1111 */
  '\xD2', /* 41 = 1112 */
  '\xD0', /* 42 = 1120 */
  '\xD0', /* 43 = 1121 */
  '\xD7', /* 44 = 1122 */
  '\xCD', /* 45 = 1200 */
  '\x20', /* 46 = 1201 */
  '\x20', /* 47 = 1202 */
  '\x20', /* 48 = 1210 */
  '\x20', /* 49 = 1211 */
  '\x20', /* 50 = 1212 */
  '\x20', /* 51 = 1220 */
  '\x20', /* 52 = 1221 */
  '\x20', /* 53 = 1222 */
  '\xCD', /* 54 = 2000 */
  '\xB8', /* 55 = 2001 */
  '\xBB', /* 56 = 2002 */
  '\xBE', /* 57 = 2010 */
  '\xB5', /* 58 = 2011 */
  '\x20', /* 59 = 2012 */
  '\xBC', /* 60 = 2020 */
  '\x20', /* 61 = 2021 */
  '\xB9', /* 62 = 2022 */
  '\x20', /* 63 = 2100 */
  '\x20', /* 64 = 2101 */
  '\x20', /* 65 = 2102 */
  '\x20', /* 66 = 2110 */
  '\x20', /* 67 = 2111 */
  '\x20', /* 68 = 2112 */
  '\x20', /* 69 = 2120 */
  '\x20', /* 70 = 2121 */
  '\x20', /* 71 = 2122 */
  '\xCD', /* 72 = 2200 */
  '\xD1', /* 73 = 2201 */
  '\xCB', /* 74 = 2202 */
  '\xCF', /* 75 = 2210 */
  '\xD8', /* 76 = 2211 */
  '\x20', /* 77 = 2212 */
  '\xCA', /* 78 = 2220 */
  '\x20', /* 79 = 2221 */
  '\xCE'  /* 80 = 2222 */
};

PTGNode print_rules( void ) {
  TableRule Rule;
  PTGNode rv = PTGNULL, nptg;

  connect_rules();
  for ( Rule = TableRules; Rule != 0; Rule = Rule->next ) {
	int i = Rule->Width * Rule->Height;
	if ( i > 0 ) {
	  for ( i--; i >= 0; i-- ) {
		Rule->rule[i] = scrchar[Rule->rule[i]];
	  }
	  nptg = Rule->Vertical ?
		print_vword(Rule->rule, Rule->Row, Rule->Col, Rule->Attr) :
		PTGString( Rule->Row, Rule->Col, Rule->Attr, PTGAsIs(Rule->rule) );
	  rv = PTGSeq(rv, nptg);
	}
  }
  return rv;
}
