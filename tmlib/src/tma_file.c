/* This file provides the file input support for TMCALGO R2 */
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "nortlib.h"
#include "tma.h"
#include "cmdalgo.h"
char rcsid_tma_file_c[] = 
  "$Header$";

static const char *yy_filename;
static int yy_lineno;
static char *yy_text;
static long int yy_val;

static void synt_err( char *txt ) {
  nl_error( 2, "%s:%d %s", yy_filename, yy_lineno, txt );
}

#define TK_NUMBER 256
#define TK_TMCCMD 257
#define TK_QSTR 258
#define KW_VALIDATE 259 
#define TK_NAME 260
#define TK_EOF 261
#define TK_ERR 262
#define KW_AND 263
#define KW_OR 264
#define KW_HOLD 265
#define KW_ELSE 266
#define KW_RESUME 267

static int lexbufsize;
static int lexbufpos;

static int buffer_char( char c ) {
  if ( lexbufpos >= lexbufsize ) {
	if ( lexbufsize == 0 ) lexbufsize = 128;
	else lexbufsize *= 2;
	yy_text = realloc( yy_text, lexbufsize );
	if ( yy_text == 0 ) {
	  synt_err( "Out of memory reading tma file" );
	  return 1;
	}
  }
  yy_text[ lexbufpos++ ] = c;
  return 0;
}

#define BEGIN_BUFFER lexbufpos = 0
#define BUFFER_CHAR(c) if ( buffer_char( c ) ) return TK_ERR
#define END_BUFFER BUFFER_CHAR(0)

/* yylex() recognizes the following tokens
  [ \t]* nothing
  [\n] increment yy_lineno
  "#.*$" nothing
  TK_NUMBER [0-9]+
  TK_TMCCMD ">.*$"
  TK_QSTR "..."
  KW_VALIDATE 
  TK_NAME [A-Z_][A-Z_0-9]*
  TK_EOF  EOF
  ':'
  ';'
  '+'
*/
static int yyunlexed = 0;
static void yyunlex( int token ) {
  if ( yyunlexed )
	nl_error( 4, "Cannot yyunlex two tokens" );
  yyunlexed = token;
}

static int yylex( FILE *fp ) {
  int c;

  if ( yyunlexed ) {
	c = yyunlexed;
	yyunlexed = 0;
	return c;
  }
  for (;;) {  
	switch ( c = getc(fp) ) {
	  case EOF:	return TK_EOF;
	  case '\n': yy_lineno++; break;
	  case ':': return ':';
	  case ';': return ';';
	  case '+': return '+';
	  case '#':
		for (;;) {
		  c = getc( fp );
		  if ( c == '\n' ) {
			ungetc( c, fp );
			break;
		  } else if ( c == EOF ) break;
		}
		break;
	  case '>':
		BEGIN_BUFFER;
		BUFFER_CHAR( c );
		for (;;) {
		  c = getc(fp);
		  if ( c != ' ' && c != '\t' ) break;
		}
		while ( c != '\n' && c != EOF ) {
		  BUFFER_CHAR( c );
		  c = getc(fp);
		}
		BUFFER_CHAR( '\n' );
		END_BUFFER;
		if ( c == '\n' ) ungetc( c, fp );
		return TK_TMCCMD;
	  case '"':
		BEGIN_BUFFER;
		BUFFER_CHAR( c );
		for (;;) {
		  c = getc(fp);
		  if ( c == '"' ) break;
		  else if ( c == '\\' ) c = getc(fp);
		  if ( c == '\n' || c == EOF ) {
			synt_err( "Runaway quote" );
			return TK_ERR;
		  }
		  BUFFER_CHAR( c );
		}
		END_BUFFER;
		return TK_QSTR;
	  default:
		if ( isspace( c ) ) break;
		else if ( isdigit(c) ) {
		  yy_val = 0;
		  do {
			yy_val = yy_val*10 + c - '0';
			c = getc(fp);
		  } while ( isdigit(c) );
		  ungetc( c, fp );
		  return TK_NUMBER;
		} else if ( isalpha(c) || c == '_' ) {
		  BEGIN_BUFFER;
		  do {
			BUFFER_CHAR(c); c = getc(fp);
		  } while( isalnum(c) || c == '_' );
		  ungetc( c, fp );
		  END_BUFFER;
		  if ( stricmp( yy_text, "validate" ) == 0 )
			return KW_VALIDATE;
		  else if ( stricmp( yy_text, "and" ) == 0 )
			return KW_AND;
		  else if ( stricmp( yy_text, "or" ) == 0 )
			return KW_OR;
		  else if ( stricmp( yy_text, "hold" ) == 0 )
			return KW_HOLD;
		  else if ( stricmp( yy_text, "else" ) == 0 )
			return KW_ELSE;
		  else if ( stricmp( yy_text, "resume" ) == 0 )
			return KW_RESUME;
		  return TK_NAME;
		} else {
		  synt_err( "syntax error" );
		  return TK_ERR;
		}
	}
  }
}

typedef struct command_st {
  char *cmd;
  struct command_st *next;
} command_t;

static int tma_strdup( command_t **ptr, const char *str, command_t *next ) {
  command_t *cmdl;
  cmdl = malloc( sizeof(command_t) );
  if ( cmdl != 0 ) {
	cmdl->cmd = strdup( str );
	if ( cmdl->cmd != 0 ) {
	  cmdl->next = next;
	  *ptr = cmdl;
	  return 0;
	}
	free( cmdl );
  }
  while ( next != 0 ) {
	cmdl = next;
	next = next->next;
	if ( cmdl->cmd ) free(cmdl->cmd);
	free(cmdl);
  }
  synt_err( "Out of memory reading tma file" );
  return 1;
}

static slurp_val *find_slurp( const char *statename ) {
  int i;
  if (statename[0] == '_') statename++;
  for ( i = 0; slurp_vals[i].state != 0; i++ ) {
	if ( strcmp( statename, slurp_vals[i].state ) == 0 ) {
	  return &slurp_vals[i];
	}
  }
  return NULL;
}

static long int last_time;
/* returns non-zero on error. dt=-1 and cmd = NULL on EOF.
   otherwise cmd is assigned a newly allocated string.
   Checks ">" command syntax, validate validity,
   time monotonicity. Supports '>', '"' and Validate
   commands plus # comments
	  timecommand : timespec command
	    : command
	  timespec : delta time
	  delta :
	    : '+'
	  time : TK_NUMBER
	    : time ':' TK_NUMBER
	  command : TK_TMCCMD
	    : TK_QSTR ';'
		: KW_VALIDATE TK_NAME ';'
		: KW_HOLD [ KW_AND KW_VALIDATE TK_NAME ]
			';' | KW_OR TK_NUMBER ( ';' | KW_ELSE command )
		: KW_RESUME TK_NAME ';'
*/

/* returns non-zero on error (and EOF is an error!) */
static int read_a_cmd( FILE *fp, command_t **cmdl, const char *mycase ) {
  slurp_val *sv;
  int token;

  token = yylex(fp);
  switch ( token ) {
	case TK_TMCCMD:
	  { int oldresp = set_response( 1 );
		int rv;
		if ( yy_text[1] == '_' )
		  rv = ci_sendcmd( yy_text+2, 1 );
		else rv = ci_sendcmd( yy_text+1, 1 );
		set_response( oldresp );
		if ( rv >= CMDREP_SYNERR ) {
		  synt_err( "Syntax Error reported by command server" );
		  return 1;
		}
	  }
	  return tma_strdup( cmdl, yy_text, NULL );
	case TK_QSTR:
	  if ( tma_strdup( cmdl, yy_text, NULL ) == 0 ) {
		if ( yylex(fp) == ';' ) return 0;
		synt_err( "Semicolon required after quoted string" );
	  }
	  return 1;
	case KW_VALIDATE:
	  token = yylex( fp );
	  if ( token != TK_NAME ) {
		synt_err( "Expected State Name after validate" );
		return 1;
	  }
	  sv = find_slurp( yy_text );
	  if ( sv == NULL ) synt_err( "Unknown state in validate" );
	  else if ( yylex(fp) != ';' )
		synt_err( "Expected ';' after validate <state>" );
	  else return tma_strdup( cmdl, sv->cmdstr, NULL );
	  return 1;
	case KW_HOLD:
	  token = yylex(fp);
	  { int valcase = 0;
		long int timeout = -1;
		command_t *cmd_else = NULL;

		if ( token == KW_AND ) {
		  if ( yylex(fp) != KW_VALIDATE ) {
			synt_err( "Expected Validate after AND" );
			return 1;
		  }
		  if ( yylex(fp) != TK_NAME ) {
			synt_err( "Expected state name after AND VALIDATE" );
			return 1;
		  }
		  sv = find_slurp( yy_text );
		  if ( sv == NULL ) {
			synt_err( "Unknown state in Hold and Validate" );
			return 1;
		  }
		  if ( sscanf( sv->cmdstr+1, "%d", &valcase ) != 1 ) {
			synt_err( "Error scanning valcase" );
			return 1;
		  }
		  token = yylex(fp);
		}
		if ( token != ';' ) {
		  if ( token == KW_OR ) {
			if ( yylex(fp) == TK_NUMBER ) {
			  timeout = yy_val;
			  token = yylex(fp);
			  if ( token == KW_ELSE ) {
				if ( read_a_cmd( fp, &cmd_else, mycase ) )
				  return 1;
			  } else if ( token != ';' ) {
				synt_err( "Expected ';' or ELSE after OR <number>" );
				return 1;
			  }
			} else {
			  synt_err( "Expected a NUMBER after OR" );
			  return 1;
			}
		  } else {
			synt_err( "Expected OR or ';' after HOLD" );
			return 1;
		  }
		}
		{ int n_skip = 1;
		  command_t *cl;
		  char buf[80];
		  
		  for ( cl = cmd_else; cl != 0; cl = cl->next ) n_skip++;
		  sprintf( buf, "?%d,%ld,%d,%s", n_skip, timeout,
					valcase, mycase );
		  return tma_strdup( cmdl, buf, cmd_else );
		}
	  }
	case KW_RESUME:
	  if ( yylex(fp) != TK_NAME )
	    synt_err( "Expecting state name after RESUME" );
	  else {
	    sv = find_slurp( yy_text );
	    if ( sv == NULL )
	      synt_err( "Unknown state after RESUME" );
	    else {
	      char const * s;
	      for ( s = sv->cmdstr; *s; s++ ) {
		if ( *s == 'R' )
		  return tma_strdup( cmdl, s, NULL );
	      }
	      synt_err( "Resume code not found" );
	    }
	  }
	  return 1;
	default:
	  synt_err( "Expected Command" );
	  return 1;
  }
}

static int read_a_tcmd( FILE *fp, const char *mycase,
	  long int *dtp, command_t **cmdl ) {
  int token;
  int delta = 0;
  long int dt = 0;

  *cmdl = NULL;  
  token = yylex( fp );
  switch ( token ) {
	case '+':
	  delta = 1;
	  token = yylex( fp );
	  if ( token != TK_NUMBER ) {
		synt_err( "Expected Number after +" );
		return 1;
	  }
	case TK_NUMBER:
	  for (;;) {
		dt = dt * 60 + yy_val;
		token = yylex( fp );
		if ( token != ':' ) break;
		token = yylex( fp );
		if ( token != TK_NUMBER ) {
		  synt_err( "Expected Number after :" );
		  return 1;
		}
	  }
	  if ( delta ) last_time += dt;
	  else if ( last_time > dt ) {
		synt_err( "Specified absolute time earlier than previous time" );
		return 1;
	  } else last_time = dt;
	  break;
	case TK_EOF:
	  *dtp = -1;
	  return 0;
	default:
	  break;
  }
  *dtp = last_time;
  yyunlex(token);
  return read_a_cmd( fp, cmdl, mycase );
}

static void free_tmacmds( tma_ifile *spec ) {
  tma_state *cmd;

  assert( spec->cmds != 0 );
  for ( cmd = spec->cmds; cmd->cmd != 0; cmd++ )
	free( (char *)(cmd->cmd) );
  free( spec->cmds );
  spec->cmds = NULL;
}

/* for read_tmafile() I need to build an array of tma_state's 
with pointers to a bunch of strings. I'll use the obvious 
approach of malloc/realloc on the tma_state array and simply use 
malloc/free on the strings. Returns 0 on success.
*/
static int read_tmafile( tma_ifile *spec, FILE *fp ) {
  int max_cmds = 32, n_cmds = 0;
  slurp_val *sv;
  const char *mycase;

  spec->cmds = malloc( max_cmds * sizeof( tma_state ) );
  if ( spec->cmds == 0 ) {
	nl_error( 2, "Out of memory reading tma file" );
	return 1;
  }
  sv = find_slurp( spec->statename );
  if ( sv == NULL ) {
    nl_error( 2, "State name '%s' not found in slurp_vals",
      spec->statename );
    free_tmacmds( spec );
    return 1;
  }
  mycase = sv->cmdstr + 1;
  yy_lineno = 1;
  yy_filename = spec->filename;
  last_time = 0;
  for (;;) {
    command_t *cmdl, *old_cmdl;
    long int dt;

    spec->cmds[n_cmds].dt = -1;
    spec->cmds[n_cmds].cmd = NULL;
    if ( read_a_tcmd( fp, mycase, &dt, &cmdl ) ) {
      /* Syntax or other error */
      free_tmacmds( spec );
      return 1;
    }
    if ( cmdl == 0 ) break; /* EOF */
    while ( cmdl != NULL ) {
      spec->cmds[n_cmds].dt = dt;
      spec->cmds[n_cmds].cmd = cmdl->cmd;
      if ( ++n_cmds == max_cmds ) {
	tma_state *ncmds;
	max_cmds *= 2;
	ncmds = realloc( spec->cmds, max_cmds * sizeof( tma_state ) );
	if ( ncmds == 0 ) {
	  if ( cmdl->cmd != 0 ) {
	    free(cmdl->cmd);
	    spec->cmds[n_cmds-1].cmd = NULL;
	  }
	  free_tmacmds( spec );
	  nl_error( 2, "Out of memory reading tma file" );
	  return 1;
	} else spec->cmds = ncmds;
      }
      old_cmdl = cmdl;
      cmdl = cmdl->next;
      free( old_cmdl );
    }
  }
  return 0;
}

void tma_read_file( tma_ifile *ifilespec ) {
  FILE *fp;
  tma_state *cmds;
  long int modtime = 0;
  struct stat buf;

  assert( ifilespec != 0 && ifilespec->filename != 0 );
  assert( ifilespec->statename != 0 );
  /* check modtime of the file. If newer, free cmds */
  if ( stat( ifilespec->filename, &buf ) == -1 )
	modtime = -1;
  else modtime = buf.st_mtime;
  if ( modtime != ifilespec->modtime && ifilespec->cmds != 0 )
	free_tmacmds( ifilespec );
  if ( modtime != ifilespec->modtime && ifilespec->cmds == 0 ) {
	fp = fopen( ifilespec->filename, "r" );
	if ( fp == 0 ) {
	  nl_error( 1, "Unable to open algo file %s for state %s", 
					ifilespec->filename, ifilespec->statename );
	} else {
	  nl_error( -2, "Reading algo file %s", ifilespec->filename );
	  read_tmafile( ifilespec, fp );
	  fclose( fp );
	}
	ifilespec->modtime = modtime;
  }
  cmds = ifilespec->cmds;
  if ( cmds == 0 ) cmds = ifilespec->def_cmds;
  tma_init_state( ifilespec->partno, cmds, ifilespec->statename );
}
