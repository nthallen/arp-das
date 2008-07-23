%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <assert.h>
  #include "nortlib.h"
  #include "rational.h"
  #include "tmcstr.h"
  #include "calibr.h"
  #include "tmc.h"
  #include "yytype.h"
  char tmc_y_rcsid[] =
	"$Id$"
	"$Version: V1R4beta $";
  
  #define yyerror(x) compile_error(2, x)
  static unsigned int decl_type = NMTYPE_DATUM;
%}
%token KW_ADDRESS
%token KW_CALIBRATION
%token <l.pretext> KW_CASE
%token <l.pretext> KW_CHAR
%token KW_COLLECT
%token KW_CONVERT
%token KW_ICONVERT
%token <l.pretext> KW_DEFAULT
%token KW_DEPENDING
%token KW_DISPLAY
%token <l.pretext> KW_DO
%token <l.pretext> KW_DOUBLE
%token <l.pretext> KW_ELSE
%token <l.pretext> KW_FLOAT
%token <l.pretext> KW_FOR
%token KW_GROUP
%token KW_HZ
%token <l.pretext> KW_IF
%token KW_INITFUNC
%token <l.pretext> KW_INT
%token KW_INVALIDATE
%token <l.pretext> KW_LONG
%token KW_MAXCOLS
%token KW_MINCOLS
%token KW_ON
%token KW_ONCE
%token <l.pretext> KW_RETURN
%token <l.pretext> KW_SHORT
%token <l.pretext> KW_SIGNED
%token KW_STATE
%token <l.pretext> KW_STRUCT
%token <l.pretext> KW_SWITCH
%token KW_SYNCHRONOUS
%token KW_TEXT
%token KW_TOLERANCE
%token KW_TM
%token <l.pretext> KW_TYPEDEF
%token <l.pretext> KW_UNION
%token <l.pretext> KW_UNSIGNED
%token KW_VALIDATE
%token <l.pretext> KW_WHILE
%token <l.pretext> TK_CHAR_CONST
%token <l.pretext> TK_DEREF
%token <l.intval> TK_INTEGER_CONST
%token <l.pretext> TK_NAME
%token <l.pretext> TK_TYPE_NAME
%token <l.pretext> TK_OPER_PUNC
%token <l.pretext> TK_REAL_CONST
%token <l.pretext> TK_STRING_LITERAL
%token UN_HOUR
%token UN_MINUTE
%token UN_SAMPLE
%token UN_SECOND
%type <statval> program
%type <statval> progitem
%type <statval> nontm_decl
%type <statval> tl_statement
%type <nlval> grouplist
%type <statval> statement
%type <l.intval> validator
%type <statval> block
%type <statval> statements
%type <statval> namelist
%type <depval> depname
%type <statval> opt_expr
%type <statval> opt_else
%type <statval> expression
%type <statval> expr_piece
%type <statval> reference
%type <statval> name_ref
%type <statval> derefs
%type <statval> deref
%type <l.pretext> oper_punc
%type <l.pretext> constant
%type <tmtval> tmtyperules
%type <tmtval> tmtyperulelist
%type <nmval> collect_rule
%type <ratval> rate
%type <ratval> tm_rate
%type <ratval> time_unit
%type <ratval> rational
%type <statval> declarations
%type <statval> declaration
%type <statval> declaration1
%type <decllist> declarators
%type <decllist> declarator1
%type <declval> declarator
%type <typeparts> array_decorations
%type <typeparts> typeparts
%type <struct_union> struct_union
%type <typeparts> integertypes
%type <typeparts> integertype
%type <sttval> statedef
%type <plval> pairs
%type <doubval> pair_num
%type <l.pretext> typedef
%type <cvtval> cvtfunc
%start starter
%%
starter : program { program = $1.first; }
	;
program : { initstat(&$$, NULL); }
	| program progitem {
		$$ = $1;
		catstat(&$$, &$2);
	  }
	;
progitem : nontm_decl {
		initstat(&$$, NULL);
		print_stat($1.first);
	  }
	| KW_TM tm_rate declaration ';' {
		struct declrtor *decl;
		struct tmalloc *tma;
		int inttype;

		assert($3.first->type == STATPC_DECLS);
		assert($3.first->next == NULL);

		/* Move pretext into typeparts of the first declaration */
		decl = $3.first->u.decls;
		initstat(&$$, newstpctext($<l.pretext>1));
		catstat(&$$, &decl->typeparts);
		decl->typeparts = $$;
		
		inttype = decl->type;
		if ( TYPE_INTEGRAL(inttype) &&
			  ! (inttype & (INTTYPE_CHAR|INTTYPE_SHORT|INTTYPE_LONG)
			  ) )
		  compile_error( 1,
			"TM frame definition not 32-bit safe"
			);
		
		/* Move size and rate into the tmalloc for each declrtor */
		for (decl = $3.first->u.decls; decl != NULL; decl = decl->next) {
		  tma = nr_tmalloc(decl->nameref);
		  tma->size = decl->size;
		  tma->rate = $2;
		}

		/* discard KW_TM and rate */
		initstat(&$$, newstpc(STATPC_TLDECLS));
		$$.first->u.stat = $3;
	  }
	| statedef ')' ';' { initstat(&$$, NULL); }
	| KW_TM KW_SYNCHRONOUS ';' {
		Synchronous = 1;
		initstat(&$$, newstpc(STATPC_TLDECLS));
		initstat(&$$.first->u.stat, newstpctext($<l.pretext>1));
	  }
	| KW_TM KW_SYNCHRONOUS KW_TOLERANCE '=' TK_INTEGER_CONST ';' {
		Synchronous = 1;
		SynchTolerance = $5;
		initstat(&$$, newstpc(STATPC_TLDECLS));
		initstat(&$$.first->u.stat, newstpctext($<l.pretext>1));
	  }
	| KW_TM KW_MAXCOLS '=' TK_INTEGER_CONST ';' {
		BPMFmax = $4;
		initstat(&$$, newstpc(STATPC_TLDECLS));
		initstat(&$$.first->u.stat, newstpctext($<l.pretext>1));
	  }
	| KW_TM KW_MINCOLS '=' TK_INTEGER_CONST ';' {
		BPMFmin = $4;
		initstat(&$$, newstpc(STATPC_TLDECLS));
		initstat(&$$.first->u.stat, newstpctext($<l.pretext>1));
	  }
	| KW_TM TK_STRING_LITERAL TK_NAME TK_INTEGER_CONST ';' {
		add_ptr_proxy($<l.toktext>2, $<l.toktext>3, $4);
		initstat(&$$, NULL);
	  }
    | KW_TM KW_INITFUNC tl_statement {
        catstat(&initprog, $3);
      }
	| tl_statement {
		struct statpc *spc;
		
		switch ($1.first->type) {
		  case STATPC_DEPEND:
			initstat(&$$, newstpc(STATPC_EXTRACT));
			$$.first->u.stat = $1;
			break;
		  case STATPC_VALID:
		  case STATPC_INVALID:
			if ($1.first->u.nameref->type == NMTYPE_STATE) {
			  initstat(&$$, NULL);
			  catstatpc(&initprog, $1.first);
			} else $$ = $1;
			break;
		  default:
			initstat(&$$, newstpc(STATPC_EXTRACT));
			spc = newdepend();
			spc->u.dep.stat = $1;
			initstat(&$$.first->u.stat, spc);
			break;
		}
	  }
	| KW_COLLECT TK_NAME block {
		struct nm *datum;

		datum = find_ref($<l.toktext>2, 0);
		if (datum->type != NMTYPE_TMDATUM)
		  compile_error(1, "Cannot collect non-TM data");
		else {
		  if (datum->u.tmdecl->collect.first != NULL)
			compile_error(1, "Attempted re-collection of %s", datum->name);
		  else {
			initstat(&datum->u.tmdecl->collect, newstpctext($<l.pretext>1));
			catstat(&datum->u.tmdecl->collect, &$3);
		  }
		}
		initstat(&$$, NULL);
	  }
	| KW_COLLECT TK_NAME '=' expression ';' {
		struct nm *datum;
		struct sttmnt *s;

		datum = find_ref($<l.toktext>2, 0);
		if (datum->type != NMTYPE_TMDATUM)
		  compile_error(1, "Cannot collect non-TM data");
		else {
		  s = &datum->u.tmdecl->collect;
		  if (s->first != NULL)
			compile_error(1, "Attempted re-collection of %s", datum->name);
		  else {
			initstat(s, newstpctext($<l.pretext>1));
			catstatpc(s, find_ref_spc($<l.toktext>2, 0));
			catstattext(s, $<l.pretext>3);
			catstat(s, &$4);
			catstattext(s, $<l.pretext>5);
		  }
		}
		initstat(&$$, NULL);
	  }
	| KW_GROUP TK_NAME '(' grouplist ')' block {
		struct nm *grp;
		struct nmlst *nl;

		grp = find_ref($<l.toktext>2, 1);
		grp->type = NMTYPE_GROUP;
		grp->u.grpd = new_memory(sizeof(struct grpdef));
		grp->u.grpd->tmdef = new_tmalloc(grp);
		grp->u.grpd->grpmems = nl = $4;
		for (; nl != NULL; nl = nl->prev) {
		  if (nl->names->type == NMTYPE_UNDEFINED)
			compile_error(2, "Group member %s undefined", nl->names->name);
		  else if (nl->names->type != NMTYPE_TMDATUM)
			compile_error(2, "Group member %s not TM datum", nl->names->name);
		  else {
			if (nl->names->u.tmdecl->collect.first != NULL)
			  compile_error(2, "Group member %s has its own collection rule",
				nl->names->name);
			else nl->names->u.tmdecl->collect = $6;
		  }
		}
		initstat(&$$, NULL);
		/* discard text */
	  }
	| KW_ADDRESS TK_NAME TK_INTEGER_CONST ';' {
		struct nm *datum;
		struct declrtor *decl;
		
		initstat(&$$, NULL);
		datum = find_name($<l.toktext>2, 0);
		if (datum == NULL)
		  compile_error(1, "Undefined datum %s", $<l.toktext>2);
		else switch (datum->type) {
		  case NMTYPE_DATUM:
		  case NMTYPE_TMDATUM:
			decl = nr_declarator(datum);
			decl->address = $3;
			decl->flag |= DCLF_ADDRDEF;
			break;
		  default:
			compile_error(2, "Address of non-datum %s", $<l.toktext>2);
			break;
		}
	  }
	| KW_CALIBRATION '(' TK_TYPE_NAME ',' TK_TYPE_NAME ')' '{' pairs '}' {
		add_calibration(find_name($<l.toktext>3, 0),
						find_name($<l.toktext>5, 0), &$8);
		initstat(&$$, NULL);
	  }
	;
nontm_decl : declaration ';' { $$ = $1; }
	| typedef declaration ';' {
		assert($2.first->type == STATPC_DECLS);
		assert($2.first->next == NULL);
		initstat(&$$, newstpctext($<l.pretext>1));
		catstat(&$$, &$2);
	  }
	| KW_TM typedef declaration1 tmtyperules {
		struct declrtor *decl;
		struct tmtype *tmt;

		assert($3.first->type == STATPC_DECLS);
		assert($3.first->next == NULL);

		/* removed loop here since declaration => declaration1 */
		decl = $3.first->u.decls;
		assert( decl != 0 && decl->next == 0 );
		/* copy tm type definition structure */
		assert(name_test(decl->nameref, NMTEST_TYPE));
		tmt = decl->nameref->u.tmtdecl;
		tmt->dummy = $4.dummy;
		tmt->collect = $4.collect;
		tmt->convert = $4.convert;
		tmt->txtfmt = $4.txtfmt;
		/* Handle inheritance of collect and convert where not explicit */
		if (decl->tm_type != 0) {
		  if ( tmt->collect.first == 0 ) {
			tmt->collect = decl->tm_type->collect;
			tmt->dummy = decl->tm_type->dummy;
		  }
		  if ( tmt->convert == 0 )
			tmt->convert = decl->tm_type->convert;
		  if ( tmt->txtfmt == NULL )
			tmt->txtfmt = decl->tm_type->txtfmt;
		}
		if ( $4.caldefs.cvt != 0 ) {
		  assert( tmt->convert != 0 );
		  assert( tmt->convert->type == NMTYPE_TMTYPE );
		  specify_conv( tmt, $4.caldefs.cvt, 
			TYPE_INTEGRAL( tmt->convert->u.tmtdecl->decl->type ) ?
			CFLG_ICVT : CFLG_CVT );
		}
		if ( $4.caldefs.tcvt != 0 )
		  specify_conv( tmt, $4.caldefs.tcvt, CFLG_TEXT );
		/* classify_conv( tmt ); */
		initstat(&$$, newstpctext($<l.pretext>1));
		catstattext(&$$, $<l.pretext>2);
		catstat(&$$, &$3);
		/* discard KW_TM */
	  }
	;
typedef : KW_TYPEDEF { decl_type = NMTYPE_TMTYPE; $$ = $1; }
	;
tm_rate : rate { $$ = $1; decl_type = NMTYPE_TMDATUM; }
	;
grouplist : TK_NAME {
		$$ = newnamelist(NULL, find_ref($<l.toktext>1, 0));
	  }
	| grouplist ',' TK_NAME {
		$$ = newnamelist($1, find_ref($<l.toktext>3, 0));
	  }
	;
tl_statement : opt_expr ';' {
		$$ = $1;
		catstattext(&$$, $<l.pretext>2);
	  }
	| block { $$ = $1; }
	| KW_DO statement KW_WHILE '(' expression ')' ';' {
		initstat(&$$, newstpctext($1));
		catstat(&$$, &$2);
		catstattext(&$$, $3);
		catstattext(&$$, $<l.pretext>4);
		catstat(&$$, &$5);
		catstattext(&$$, $<l.pretext>6);
		catstattext(&$$, $<l.pretext>7);
	  }
	| KW_IF '(' expression ')' statement opt_else {
		initstat(&$$, newstpctext($1));
		catstattext(&$$, $<l.pretext>2);
		catstat(&$$, &$3);
		catstattext(&$$, $<l.pretext>4);
		catstat(&$$, &$5);
		catstat(&$$, &$6);
	  }
	| KW_SWITCH '(' expression ')' statement {
		initstat(&$$, newstpctext($1));
		catstattext(&$$, $<l.pretext>2);
		catstat(&$$, &$3);
		catstattext(&$$, $<l.pretext>4);
		catstat(&$$, &$5);
	  }
	| KW_WHILE '(' expression ')' statement {
		initstat(&$$, newstpctext($1));
		catstattext(&$$, $<l.pretext>2);
		catstat(&$$, &$3);
		catstattext(&$$, $<l.pretext>4);
		catstat(&$$, &$5);
	  }
	| KW_FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' statement {
		initstat(&$$, newstpctext($1));
		catstattext(&$$, $<l.pretext>2);
		catstat(&$$, &$3);
		catstattext(&$$, $<l.pretext>4);
		catstat(&$$, &$5);
		catstattext(&$$, $<l.pretext>6);
		catstat(&$$, &$7);
		catstattext(&$$, $<l.pretext>8);
		catstat(&$$, &$9);
	  }
	| KW_RETURN opt_expr ';' {
		initstat(&$$, newstpctext($1));
		catstat(&$$, &$2);
		catstattext(&$$, $<l.pretext>3);
	  }
	| KW_DEPENDING KW_ON '(' namelist ')' statement opt_else {
		$$ = $4;
		$$.first->u.dep.stat = $6;
		$$.first->u.dep.else_stat = $7;
	  }
	| validator TK_NAME ';' {
		struct nm *var;
		
		initstat(&$$, find_ref_spc($<l.toktext>2, 0));
		$$.last->type = $1;
		var = $$.last->u.nameref;
		if (name_test(var, NMTEST_VALID)) {
		  if ($1 == STATPC_INVALID) {
			struct validator *vldtr;

			/* Should test here to see if it's OK to invalidate */
			if (name_test(var, NMTEST_INVALID)) {
			  vldtr = nr_validator(var);
			  vldtr->flag |= DCLF_VALIDATE;
			} else compile_error(2, "Cannot invalidate datum %s", var->name);
		  }
		} else compile_error(2, "Cannot [in]validate non-datum %s", var->name);
	  }
	| KW_DISPLAY '(' constant ',' constant ',' constant ','
					 name_ref ')' ';' {
		struct sttmnt st;
		struct statpc *sp;
		struct declrtor *decl;
		struct nm *dat;
		
		catstattext(&initprog, "\ncdisplay");
		initstat(&$$, newstpctext("\ndisplay_"));
		
		initstat(&st, newstpctext($<l.pretext>2));
		catstattext(&st, $3);
		catstattext(&st, $<l.pretext>4);
		catstattext(&st, $5);
		catstattext(&st, $<l.pretext>6);
		catstattext(&st, $7);
		catstattext(&st, $<l.pretext>8);
		catstatpc(&initprog, common_stat(st.first));
		catstatpc(&$$, common_stat(st.first));
		
		st = $9;
		get_cfnc(&st, CFLG_TEXT);
		/* if get_cfnc doesn't return a STATPC_CONVERT,
		   it declared an error, so we don't need to
		   worry about our output */
		if ( st.last->type == STATPC_CONVERT ) {
		  sp = st.last->u.cvt.ref;
		  assert(sp->type == STATPC_REF);
		  dat = sp->u.nameref;
		  if (!name_test(dat, NMTEST_DATA))
			 compile_error(3, "Illegal type in display() call");
		  decl = nr_declarator(dat);
		  catstatpc(&$$, common_stat(decl->typeparts.first));
		  catstattext(&$$, ",");
		  catstatpc(&$$, common_stat(sp));
		  catstattext(&$$, ",");
		  sp = newstpc(STATPC_CONVERT);
		  *sp = *st.last;
		  sp->u.cvt.ref = NULL;
		  catstatpc(&initprog, sp);
		  catstat(&$$, &st);
		
		  sp = newstpctext(");");
		  catstatpc(&initprog, common_stat(sp));
		  catstatpc(&$$, common_stat(sp));
		}
	  }
	;
statement : tl_statement { $$ = $1; }
	| TK_NAME ':' statement {
		initstat(&$$, newstpctext($<l.pretext>1));
		catstattext(&$$, $<l.toktext>1);
		catstattext(&$$, $<l.pretext>2);
		catstat(&$$, &$3);
	  }
	| KW_CASE expression ':' statement {
		initstat(&$$, newstpctext($1));
		catstat(&$$, &$2);
		catstattext(&$$, $<l.pretext>3);
		catstat(&$$, &$4);
	  }
	| KW_DEFAULT ':' statement {
		initstat(&$$, newstpctext($1));
		catstattext(&$$, $<l.pretext>2);
		catstat(&$$, &$3);
	  }
	;
validator : KW_VALIDATE { $$ = STATPC_VALID; }
	| KW_INVALIDATE { $$ = STATPC_INVALID; }
	;	
block : '{' {new_scope();} declarations statements '}' {
		initstat(&$$, newstpctext($<l.pretext>1));
		catstat(&$$, &$3);
		catstat(&$$, &$4);
		catstattext(&$$, $<l.pretext>5);
		old_scope();
	  }
	;
statements : { initstat(&$$, NULL); }
	| statements statement {
		$$ = $1;
		catstat(&$$, &$2);
	  }
	;
namelist : depname {
		initstat(&$$, newdepend());
		$$.first->u.dep.vd = $1;
	  }
	| namelist ',' depname {
		$3->next = $1.first->u.dep.vd;
		$1.first->u.dep.vd = $3;
		$$ = $1;
	  }
	| rate {
		initstat(&$$, newdepend());
		$$.first->u.dep.rate = $1;
	  }
	| namelist ',' rate {
		$$ = $1;
		if ($1.first->u.dep.rate.num != 0)
		  compile_error(2, "Only one rate allowed in 'depending on'");
		else $$.first->u.dep.rate = $3;
	  }
	;
depname : TK_NAME {	$$ = newdeplst($<l.toktext>1, 0); }
	| TK_NAME KW_ONCE {	$$ = newdeplst($<l.toktext>1, DEPL_ONCE); }
	;
opt_else : { initstat(&$$, NULL); }
	| KW_ELSE statement {
		initstat(&$$, newstpctext($1));
		catstat(&$$, &$2);
	  }
	;
statedef : KW_STATE '(' TK_NAME {
		$$ = new_stateset();
		add_state($$, $<l.toktext>3);
	  }
	| statedef ',' TK_NAME {
		$$ = $1;
		add_state($$, $<l.toktext>3);
	  }
	;
opt_expr : { initstat(&$$, NULL); }
	| expression { $$ = $1; }
	;
expression : expr_piece { $$ = $1; }
	| expression expr_piece { $$ = $1; catstat(&$$, &$2); }
	| expression '?' expression ':' expression {
		$$ = $1;
		catstattext(&$$, $<l.pretext>2);
		catstat(&$$, &$3);
		catstattext(&$$, $<l.pretext>4);
		catstat(&$$, &$5);
	  }
	;
expr_piece : constant { initstat(&$$, newstpctext($1)); }
	| reference { $$ = $1; }
	| oper_punc { initstat(&$$, newstpctext($1)); }
	| '(' opt_expr ')' {
		initstat(&$$, newstpctext($<l.pretext>1));
		catstat(&$$, &$2);
		catstattext(&$$, $<l.pretext>3);
	  }
	;
reference : name_ref { $$ = $1; }
	| name_ref '.' KW_ADDRESS {
		$$ = $1;
		$$.last->type = STATPC_ADDR;
	  }
	| KW_CONVERT '(' name_ref ')' {
		$$ = $3;
		get_cfnc(&$$, CFLG_CVT);
	  }
	| KW_ICONVERT '(' name_ref ')' {
		$$ = $3;
		get_cfnc(&$$, CFLG_ICVT);
	  }
	| KW_TEXT '(' name_ref ')' {
		$$ = $3;
		get_cfnc(&$$, CFLG_TEXT);
	  }
	| name_ref derefs { $$ = $1; catstat(&$$, &$2); }
	;
name_ref : TK_NAME {
		initstat(&$$, newstpctext($<l.pretext>1));
		catstatpc(&$$, find_ref_spc($<l.toktext>1, 0));
	  }
	;
derefs : deref { $$ = $1; }
	| derefs deref { $$ = $1; catstat(&$$, &$2); }
	;
deref : '.' TK_NAME {
		initstat(&$$, newstpctext($<l.pretext>1));
		catstattext(&$$, $<l.pretext>2); /* may be NULL */
		catstattext(&$$, $<l.toktext>2);
	  }
	| TK_DEREF TK_NAME {
		initstat(&$$, newstpctext($<l.pretext>1));
		catstattext(&$$, $<l.pretext>2); /* may be NULL */
		catstattext(&$$, $<l.toktext>2);
	  }
	;	
oper_punc : TK_OPER_PUNC { $$ = $1; }
	| ']' { $$ = $<l.pretext>1; }
	| '[' { $$ = $<l.pretext>1; }
	| ',' { $$ = $<l.pretext>1; }
	| '/' { $$ = $<l.pretext>1; }
	| '=' { $$ = $<l.pretext>1; }
	| '-' { $$ = $<l.pretext>1; }
	| '+' { $$ = $<l.pretext>1; }
	;
constant : TK_INTEGER_CONST { $$ = $<l.pretext>1; }
	| TK_REAL_CONST { $$ = $1; }
	| TK_CHAR_CONST { $$ = $1; }
	| TK_STRING_LITERAL { $$ = $1; }
	;
tmtyperules : ';' { clr_tmtype(&$$); }
	| '{' tmtyperulelist '}' { $$ = $2; }
	;
tmtyperulelist : { clr_tmtype(&$$); }
	| tmtyperulelist collect_rule '{' declarations statements '}' {
		if ($1.collect.first != NULL)
		  compile_error(2, "Only one collection rule allowed");
		$$ = $1;
		$$.dummy = $2;
		initstat(&$$.collect, newstpctext($<l.pretext>3));
		catstat(&$$.collect, &$4);
		catstat(&$$.collect, &$5);
		catstattext(&$$.collect, $<l.pretext>6);
		old_scope();
	  }
	| tmtyperulelist collect_rule '=' expression ';' {
		if ($1.collect.first != NULL)
		  compile_error(2, "Only one collection rule allowed");
		$$ = $1;
		$$.dummy = $2;
		initstat(&$$.collect, newstpc(STATPC_REF));
		$$.collect.first->u.nameref = $2;
		catstattext(&$$.collect, $<l.pretext>3);
		catstat(&$$.collect, &$4);
		catstattext(&$$.collect, $<l.pretext>5);
		old_scope();
	  }
	| tmtyperulelist KW_CONVERT TK_TYPE_NAME cvtfunc ';' {
		if ($1.convert != NULL)
		  compile_error(2, "Only one conversion allowed");
		$$ = $1;
		$$.convert = find_ref($<l.toktext>3, 0);
		assert(name_test($$.convert, NMTEST_TYPE));
		if ( $4 != 0 ) {
		  /* CFLG_CVT of CFLG_ICVT depends on type defs
		     If to-type is integral, use CFLG_ICVT, else
			 CFLG_CVT.
			 Resolve above when we know the ftype, etc.
		  */
		  assert( $$.caldefs.cvt == 0 );
		  $$.caldefs.cvt = $4;
		}
	  }
	| tmtyperulelist KW_TEXT TK_STRING_LITERAL cvtfunc ';' {
		char *s;
		
		if ($1.txtfmt != NULL)
		  compile_error(2, "Only one text format allowed");
		$$ = $1;
		s = $<l.toktext>3;
		assert(*s == '\"');
		for (s++; *s != '\0'; s++);
		--s;
		assert(*s == '\"');
		*s = '\0';
		$$.txtfmt = strdup($<l.toktext>3+1);
		free_memory($<l.pretext>2);
		$$.caldefs.tcvt = $4;
	  }
	;
collect_rule : KW_COLLECT TK_NAME {
		new_scope();
		$$ = find_ref($<l.toktext>2, 1);
		$$->type = NMTYPE_DUMMY;
		/* discard collect */
		/* discard TK_NAME pretext */
	  }
	;
cvtfunc : { $$ = NULL; }
	| TK_NAME '(' ')' { $$ = mk_cvt_func( $<l.toktext>1, '(' ); }
	| TK_NAME '[' ']' { $$ = mk_cvt_func( $<l.toktext>1, '[' ); }
	| '(' ')' { $$ = mk_cvt_func( "", '(' ); }
	;
rate : rational UN_SAMPLE '/' time_unit { rdivide(&$1, &$4, &$$); }
	| rational KW_HZ { $$ = $1; }
	| rational time_unit '/' UN_SAMPLE {
		rtimes(&$1, &$2, &$$);
		rdivide(&one, &$$, &$$);
	  }
	;
time_unit : UN_SECOND { $$.num = $$.den = 1; }
	| UN_MINUTE { $$.num = 60; $$.den = 1; }
	| UN_HOUR { $$.num = 3600; $$.den = 1; }
	;
rational : TK_INTEGER_CONST {
		$$.num = $1;
		$$.den = 1;
	  }
	| TK_INTEGER_CONST '/' TK_INTEGER_CONST {
		$$.num = $1;
		$$.den = $3;
		if ($3 == 0) compile_error(2, "Zero denominator in rate");
		else rreduce(&$$);
	  }
	;
	/* statval */
declarations : { initstat(&$$, NULL); }
	| declarations declaration ';' {
		/* a declaration is a single statpc of type STATPC_DECLS
		   (declarators).
		   declarations are a string of STATPC_DECLS. There is
		   an implied ';' after each which is returned in
		   print_decl if necessary (i.e. if the declaration
		   is actually printed).
	    */
		$$ = $1;
		catstat(&$$, &$2);
		assert($$.last->type == STATPC_DECLS);
	  }
	;
	/* statval */
declaration : declarators {
		/* declaration simply promotes declarators (decllist)
		   to a statval */
		initstat(&$$, newstpc(STATPC_DECLS));
		$$.first->u.decls = $1.first;
		decl_type = NMTYPE_DATUM;
	  }
	;
	/* statval */
declaration1 : declarator1 {
		/* declaration1 simply promotes declarator1 (decllist)
		   to a statval */
		initstat(&$$, newstpc(STATPC_DECLS));
		$$.first->u.decls = $1.first;
		decl_type = NMTYPE_DATUM;
	  }
	;
	/* decllist */
declarator1 : typeparts declarator {
		$$.typeparts = $1;
		$$.first = $$.last = $2;
		$2->typeparts = $1.stat;
		$2->size *= $1.size;
		$2->type = $1.type;
		$2->tm_type = $1.tm_type;
	  }
	;
	/* decllist */
declarators : declarator1 { $$ = $1; }
	| declarators ',' declarator {
		/* I discard the comma, put it back during output if necessary */
		$$.typeparts = $1.typeparts;
		$$.first = $1.first;
		$1.last->next = $3;
		$$.last = $3;
		$3->typeparts = $$.typeparts.stat;
		$3->size *= $1.typeparts.size;
		$3->type = $1.typeparts.type;
		$3->tm_type = $1.typeparts.tm_type;
	  }
	;
	/* Missing from the declarator syntax is pointer decorations
	   and function decorations
	   Should be: [pointer dec] ( name | '(' declarator ')' )
	             ( array_decorations | function_decorations )
	   A subset of these syntaxes are useful to TMC.
	   declarator is a declval, i.e. (struct declrtor *)
	 */
declarator : TK_NAME array_decorations {
		$$ = new_memory(sizeof(struct declrtor));
		$$->next = NULL;
		initstat(&$$->typeparts, NULL); /* just to make sure */
		initstat(&$$->decl, newstpctext($<l.pretext>1));
		catstatpc(&$$->decl, find_ref_spc($<l.toktext>1, 1));
		$$->nameref = $$->decl.last->u.nameref;
		$$->size = $2.size;
		$$->tm_type = NULL;
		$$->flag = 0;
		catstat(&$$->decl, &$2.stat);
		link_declarator($$, decl_type);
	  }
	;
array_decorations : { $$.size = 1; $$.stat.first = $$.stat.last = NULL; }
	| array_decorations '[' TK_INTEGER_CONST ']' {
		$$ = $1;
		$$.size *= $3;
		catstattext(&$$.stat, $<l.pretext>2);
		catstattext(&$$.stat, $<l.pretext>3);
		catstattext(&$$.stat, $<l.pretext>4);
	  }
	;
typeparts : integertypes {
		static unsigned char intsizes[16] =
		  {2,1,2,0,4,0,4,0,2,0,2,0,0,0,0,0};
		assert($1.type < 32);
		set_typpts(&$$, $1.type, intsizes[$1.type & 0xF], NULL, NULL);
		$$.stat = $1.stat;
	  }
	| KW_FLOAT { set_typpts(&$$, INTTYPE_FLOAT, 4, $1, NULL); }
	| KW_DOUBLE { set_typpts(&$$, INTTYPE_DOUBLE, 8, $1, NULL); }
	| TK_TYPE_NAME {
		struct statpc *spc;
		struct tmtype *tmt;

		spc = find_ref_spc($<l.toktext>1, 0);
		assert(name_test(spc->u.nameref, NMTEST_TYPE));
		tmt = spc->u.nameref->u.tmtdecl;
		set_typpts(&$$, tmt->decl->type, tmt->decl->size,
					  $<l.pretext>1, tmt);
		catstatpc(&$$.stat, spc);
	  }
	| struct_union '{' declarations '}' {
		decl_type = end_st_un(&$$, &$1, $<l.pretext>2, &$3, $<l.pretext>4);
	  }
	;
struct_union : KW_STRUCT {
		decl_type = start_st_un(&$$, $1, INTTYPE_STRUCT, decl_type);
	  }
	| KW_UNION {
		decl_type = start_st_un(&$$, $1, INTTYPE_UNION, decl_type);
	  }
	;
integertypes : integertype { $$ = $1; }
	| integertypes integertype {
		$$.stat = $1.stat;
		catstat(&$$.stat, &$2.stat);
		$$.type |= $2.type;
	  }
	;
integertype : KW_CHAR { int_type(&$$, $1, INTTYPE_CHAR); }
	| KW_INT { int_type(&$$, $1, INTTYPE_INT); }
	| KW_LONG { int_type(&$$, $1, INTTYPE_LONG); }
	| KW_SHORT { int_type(&$$, $1, INTTYPE_SHORT); }
	| KW_SIGNED { int_type(&$$, $1, 0); }
	| KW_UNSIGNED { int_type(&$$, $1, INTTYPE_UNSIGNED); }
	;
pairs : pair_num ',' pair_num {
		$$.pairs = NULL;
		add_pair(&$$, $1, $3);
	  }
	| pairs ',' pair_num ',' pair_num { add_pair(&$$, $3, $5); }
	;
pair_num : TK_INTEGER_CONST { $$ = (double) $1; }
	| TK_REAL_CONST { $$ = strtod($<l.toktext>1, (char**)NULL); }
	| '-' pair_num { $$ = -$2; }
	| '+' pair_num { $$ = $2; }
	;
