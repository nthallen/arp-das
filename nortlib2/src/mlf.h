#ifndef MLF_H_INCLUDED
#define MLF_H_INCLUDED
#include <stdio.h>
#include <limits.h>

typedef struct {
  int index;
  char *s;
} mlf_elt_t;

typedef struct {
  mlf_elt_t *flvl;
  int n_levels;
  int n_files;
  char fpath[PATH_MAX];
  char *fsuffix;
  int flags;
  unsigned long index;
} mlf_def_t;
#define MLF_INC_FIRST 1
#define MLF_WRITING 2
#define MLF_INITIALIZE 4
#define MLF_MAX_LEVELS 10

typedef struct {
  mlf_def_t *mlf;
  int *ntup;
  char *base;
  char *suffix;
} mlf_ntup_t;

FILE *mlf_next_file( mlf_def_t *mlf );
mlf_def_t *mlf_init( int n_levels, int n_files, int writing,
	char *fbase, char *fsuffix, char *config );
mlf_ntup_t *mlf_convert_fname( mlf_def_t *mlf, char *fbase, char *fname );
void mlf_free_mlfn( mlf_ntup_t *mlfn );
void mlf_set_ntup( mlf_def_t *mlf, mlf_ntup_t *mlfn );
int mlf_compare( mlf_def_t *mlf, mlf_ntup_t *mlfn );
void mlf_set_index( mlf_def_t * mlf, unsigned long index );

#endif
