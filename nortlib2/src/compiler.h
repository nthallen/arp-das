/* compiler.h
 * $Log$
 */
#ifndef _COMPILER_H_INCLUDED
#define _COMPILER_H_INCLUDED

#ifndef _LLOFSTR_INCLUDED
  #include "ll_of_str.h"
#endif

extern FILE *ofile, *yyin;
extern char *input_filename, *output_filename;
extern int input_linenumber, error_level;
extern char *opt_string;
extern ll_of_str input_files;

#define OPT_COMPILER_INIT "qkwvo:"

/* CO_IGN_WARN and CO_KEEP_OUTPUT are common. CO_CUSTOM provides
   a hook for other options. Any bits higher than CO_CUSTOM are
   available for an individual developer.
*/
extern short compile_options;
#define CO_IGN_WARN 1
#define CO_KEEP_OUTPUT 2
#define CO_CUSTOM 4

int compile_error(int level, char *format, ...);
FILE *open_output_file(char *name);
FILE *open_input_file(char *name);
int compile_init_options(int argc, char **argv, char *extension);
#ifdef yywrap
  #undef yywrap
#endif
int yywrap(void);
int yylex(void);
int yyparse(void);
extern int yydebug;

#if defined __386__
#  pragma library (nortlib3r)
#elif defined __SMALL__
#  pragma library (nortlibs)
#elif defined __COMPACT__
#  pragma library (nortlibc)
#elif defined __MEDIUM__
#  pragma library (nortlibm)
#elif defined __LARGE__
#  pragma library (nortlibl)
#endif

#endif
