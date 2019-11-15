/** @file tmcmain.c
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "nortlib.h"
#include "nl_assert.h"
#include "tmc.h"

int (*nl_error)(int level, const char *format, ...) = compile_error;

FILE *vfile = NULL, *dacfile = NULL, *addrfile = NULL;

#define COLLECT_SKELETON "colmain.skel"
#define EXTRACT_SKELETON "extmain.skel"

#ifdef __USAGE
tmc Telemetry Compiler Version 1 Revision 13

%C	[options] [files]
	-c             Generate collection rules
	-C             Print information on Conversions
	-d             Print Data Definitions
	-D filename    Create dac file
	-h             Show this help message
	-H filename    Create .h file for addresses
	-i             Print Steps in compilation progress
	-k             Keep output file (.c) even on error
	-m             Do not generate main() function
	-o filename    Send C output to file
	-p             Print PCM (TM Format) Definition
	-v             Produce very verbose output (-Cdps)
	-V filename    Redirect verbose output to file
	-s             Print frame statistics
	-w             Give error return on warnings
	
Copyright 2001-2019 by the President and Fellows of Harvard College
#endif

static void print_usage(int argc, char **argv) {
  printf("tmc Telemetry Compiler Version 1 Revision 13\n\n");

  printf("%s [options] [files]\n",argv[0]);
  printf("%s\n", "  -c             Generate collection rules");
  printf("%s\n", "  -C             Print information on Conversions");
  printf("%s\n", "  -d             Print Data Definitions");
  printf("%s\n", "  -D filename    Create dac file");
  printf("%s\n", "  -h             Show this help message");
  printf("%s\n", "  -H filename    Create .h file for addresses");
  printf("%s\n", "  -i             Print Steps in compilation progress");
  printf("%s\n", "  -k             Keep output file (.c) even on error");
  printf("%s\n", "  -m             Do not generate main() function");
  printf("%s\n", "  -o filename    Send C output to file");
  printf("%s\n", "  -p             Print PCM (TM Format) Definition");
  printf("%s\n", "  -v             Produce very verbose output (-Cdps)");
  printf("%s\n", "  -V filename    Redirect verbose output to file");
  printf("%s\n", "  -s             Print frame statistics");
  printf("%s\n", "  -w             Give error return on warnings");

  printf("\n%s\n", "Copyright 2001-2019 by the President and Fellows of Harvard College");
}

const char *opt_string = OPT_COMPILER_INIT "cCdD:hH:impsV:";

static void main_args(int argc, char **argv) {
  int c;
  
  opterr = 0;
  optind = OPTIND_RESET;
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'h':
        print_usage(argc, argv);
        exit(0);
      default:
        break;
    }
  }
  compile_init_options(argc, argv, ".c");
  opterr = 0;
  optind = OPTIND_RESET;
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'c':	compile_options |= CO_COLLECT;	break;
      case 'm': compile_options |= CO_NO_MAIN; break;
      case 'v':
        setshow(TM_PDEFS);
        setshow(TM_DEFS);
        setshow(TM_STEPS);
        setshow(TM_STATS);
        setshow(CONVERSIONS);
        break;
      case 'p': setshow(TM_DEFS); break;
      case 'd': setshow(TM_PDEFS); break;
      case 'i': setshow(TM_STEPS); break;
      case 's': setshow(TM_STATS); break;
      case 'C': setshow(CONVERSIONS); break;
      case 'V':
        vfile = open_output_file(optarg);
        break;
      case 'D':
        dacfile = open_output_file(optarg);
        break;
      case 'H':
        addrfile = open_output_file(optarg);
        break;
      case '?':
        compile_error(3, "Unrecognized option -%c", optopt);
    }
  }
  if (vfile == NULL) vfile = stdout;
}

int main(int argc, char **argv) {
  unsigned int errlevel;

  main_args(argc, argv);
  Skel_open(Collecting?COLLECT_SKELETON:EXTRACT_SKELETON);
  Skel_copy(ofile, "headers", 1);
  new_scope();
  errlevel = yyparse();
  if (error_level < errlevel) error_level = errlevel;
  if (error_level == 0) {
    if (Collecting) print_recv_objs();

    /* Copy out console functions */
    Skel_copy(ofile, "console_functions", 1);
  
    generate_pcm(); /* operates on global_scope */
    
    /* copy statements from data in global_scope onto slotlist */
    if (Collecting) place_col();
    
    /* Handle top-level validations in program */
    place_valid();
    
    /* copy statements from program (_EXTRACT) to slotlist */
    place_ext();
    
    print_decls(); /* but not home row! */
    
    /* Position TM data (global_scope) in the home row */
    place_home();
    
    /* Document PCM format */
    print_pcm();
    
    /* Generate calibration conversion functions */
    declare_convs();

    /* Generate State Invalidation Functions */
    print_states();
    
    /* Generate init function and TM functions */
    print_funcs();
    
    /* Output #defines for skeleton, skeleton itself and dbr_info */
    post_processing();
  }
  if (error_level) fprintf(stderr, "Error level %d\n", error_level);
  exit(error_level);
}
