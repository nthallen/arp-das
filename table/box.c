#include <stdlib.h>
#include "err.h"

#define RL_BT 1
#define RL_TP 3
#define RL_RT  9
#define RL_LT 27

typedef struct tblrule {
  struct tblrule *next;
  int Row, Col, Length;
  int Vertical, Lines, Connected;
  int Attr;
  unsigned char *rule;
} *TableRule;

static TableRule TableRules;

void NewRule( int Row, int Col, int Length, int Vertical,
	  int Lines, int Connected, int Attr ) {
  TableRule new;
  unsigned char *rule;
  unsigned char middle;
  
  new = malloc(sizeof(struct tblrule));
  rule = malloc( Length+1 );
  if ( new == 0 || rule == 0 )
	message(DEADLY, "Out of memory in NewRule", 0, &curpos );
  new->Row = Row;
  new->Col = Col;
  new->Length = Length;
  new->Vertical = Vertical;
  new->Lines = Lines;
  new->Connected = Connected;
  new->Attr = Attr;
  if ( Vertical ) middle = (RL_TP+RL_BT)*Lines;
  else middle = (RL_LT+RL_RT)*Lines;
  { int i;
	for ( i = 0; i < Length; i++ ) rule[i] = middle;
  }
  rule[Length] = '\0';
  if ( ! Connected ) {
	if ( Length == 1 ) rule[0] = 0;
	else {
	  if ( Vertical ) {
		rule[0] = RL_BT * Lines;
		rule[Length-1] = RL_TP * Lines;
	  } else {
		rule[0] = RL_RT * Lines;
		rule[Length-1] = RL_LT * Lines;
	  }
	}
  }
  new->rule = rule;
  new->next = TableRules;
  TableRules = new;
}

static void add_bits( unsigned char *r, int i, int t, int lines ) {
  int c;
  c = (r[i]/t)%3;
  if ( c != 0 )
	message( DEADLY, "Nonzero rule element", 0, &curpos );
  r[i] += t * lines;
}

static void connect_rules( void ) {
  TableRule R, S;
  unsigned char c, cc;
  
  for ( R = TableRules; R != 0; R = R->next ) {
	if ( R->Connected ) {
	  if ( R->Vertical ) {
		for ( S = TableRules; S != 0; S = S->next ) {
		  if (! S->Vertical && S->Col <= R->Col &&
				S->Col+S->Length > R->Col ) {
			if ( S->Row == R->Row-1 ) {
			  add_bits( S->rule, R->Col-S->Col, RL_BT, R->Lines );
			} else if ( S->Row == R->Row+R->Length ) {
			  add_bits( S->rule, R->Col-S->Col, RL_TP, R->Lines );
			}
		  }
		}
	  } else {
		for ( S = TableRules; S != 0; S = S->next ) {
		  if ( S->Vertical && S->Row <= R->Row &&
				S->Row+S->Length > R->Row ) {
			if ( S->Col == R->Col-1 ) {
			  add_bits( S->rule, R->Row-S->Row, RL_RT, R->Lines );
			} else if ( S->Col == R->Col+R->Length ) {
			  add_bits( S->rule, R->Row-S->Row, RL_LT, R->Lines );
			}
		  }
		}
	  }
	}
  }
}

unsigned char scrchar[] = {
  '\x10', /*  0 = 0000 */
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
  '\x10', /* 46 = 1201 */
  '\x10', /* 47 = 1202 */
  '\x10', /* 48 = 1210 */
  '\x10', /* 49 = 1211 */
  '\x10', /* 50 = 1212 */
  '\x10', /* 51 = 1220 */
  '\x10', /* 52 = 1221 */
  '\x10', /* 53 = 1222 */
  '\xCD', /* 54 = 2000 */
  '\xB8', /* 55 = 2001 */
  '\xBB', /* 56 = 2002 */
  '\xBE', /* 57 = 2010 */
  '\xB5', /* 58 = 2011 */
  '\x10', /* 59 = 2012 */
  '\xBC', /* 60 = 2020 */
  '\x10', /* 61 = 2021 */
  '\xB9', /* 62 = 2022 */
  '\x10', /* 63 = 2100 */
  '\x10', /* 64 = 2101 */
  '\x10', /* 65 = 2102 */
  '\x10', /* 66 = 2110 */
  '\x10', /* 67 = 2111 */
  '\x10', /* 68 = 2112 */
  '\x10', /* 69 = 2120 */
  '\x10', /* 70 = 2121 */
  '\x10', /* 71 = 2122 */
  '\xCD', /* 72 = 2200 */
  '\xD1', /* 73 = 2201 */
  '\xCB', /* 74 = 2202 */
  '\xCF', /* 75 = 2210 */
  '\xD8', /* 76 = 2211 */
  '\x10', /* 77 = 2212 */
  '\xCA', /* 78 = 2220 */
  '\x10', /* 79 = 2221 */
  '\xCE'  /* 80 = 2222 */
};

void print_rules( void ) {
  TableRule Rule;

  connect_rules();
  for ( Rule = TableRules; Rule != 0; Rule = Rule->next ) {
	{ int i;
	  for ( i = 0; i < Rule->Length; i++ ) {
		Rule->rule[i] = scrchar[Rule->rule[i]];
	  }
	}
	if ( Rule->Vertical ) {
	  int i;
	  for ( i = 0; i < Rule->Length; i++ ) {
		printf( "#STRING %d %d %d \"%c\"\n",
		  Rule->Row + i, Rule->Col, Rule->Attr, Rule->rule[i] );
	  }
	} else if ( Rule->Length > 0 ) {
	  printf( "#STRING %d %d %d \"%s\"\n",
		  Rule->Row, Rule->Col, Rule->Attr, Rule->rule );
	}
  }
}
