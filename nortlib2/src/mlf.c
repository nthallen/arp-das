/* config string is of the form
  \w+/(\d\d+/(\d\d/(\d\d\.dat)?)?)?
  Or perhaps more precisely:
  <base>/
  <base>/dd+/
  <base>/dd+/dd/
  <base>/dd+/dd/dd.dat

  Todo:
	Separate out a type to hold a file as an n-tuple
	mlf_ntup_t *mlfn;
	mlf_ntup_t *mlf_convert_fname( mlf_def_t *mlf, char *fname );
	mlf_set_ntup( mlf_def_t *mlf, mlf_ntup_t *mlfn );
	mlf_compare( mlf_def_t *mlf, mlf_ntup_t *mlfn );
*/
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
#include "mlf.h"
#include "nortlib.h"

static char *mlf_strtok( char *buf, char *delset, char *delp ) {
  static char *bufp;
  int n;
  char *rbuf;
  
  if ( buf != NULL ) bufp = buf;
  n = strcspn( bufp, delset );
  *delp = bufp[n];
  bufp[n] = '\0';
  rbuf = bufp;
  bufp += n;
  if ( *delp ) bufp++;
  return rbuf;
}

/* Parses fname into an n-tuple.
  If fname == NULL or is empty, a valid zeroed n-tuple is
  returned.
*/
mlf_ntup_t *mlf_convert_fname( mlf_def_t *mlf, char *fbase, char *fname ) {
  mlf_ntup_t *mlfn;
  char *cfg;
  char *num, *s, del;
  int level;

  mlfn = new_memory( sizeof( mlf_ntup_t ) );
  mlfn->ntup = new_memory( (mlf->n_levels+1) * sizeof(int) );
  for ( level = 0; level <= mlf->n_levels; level++ )
	mlfn->ntup[level] = 0;
  mlfn->mlf = mlf;
  mlfn->base = fbase;
  mlfn->suffix = NULL;
  if ( fname == NULL || *fname == '\0' ) return mlfn;

  cfg = nl_strdup( fname );
  mlfn->base = mlf_strtok( cfg, "/", &del );
  for ( s = mlfn->base; *s; s++ )
	if ( ! isalnum(*s) && *s != '_' )
	  nl_error( 3,
		"mlf_convert_fname: illegal char '%c' in base '%s'",
		*s, mlfn->base );

  for ( level = 0; level <= mlf->n_levels; level++ ) {
	num = mlf_strtok( NULL, level < mlf->n_levels ? "/" : "/.",	&del );
	if ( num != NULL && *num != '\0' ) {
	  char *end;

	  if ( del == '/' && level == mlf->n_levels )
		nl_error( 3,
		  "Too many directory levels specified", level );
	  mlfn->ntup[level] = strtoul( num, &end, 10 );
	  if ( *end != '\0' )
		nl_error( 3,
		  "mlf_convert_fname: Subdir '%s' at level %d not numeric",
		  num, level );
	}
  }
  mlfn->suffix = mlf_strtok( NULL, "/", &del );
  for ( s = mlfn->suffix; *s; s++ )
	if ( ! isalnum(*s) && *s != '_' )
	  nl_error( 3, "mlf_convert_fname: Illegal char in suffix" );

  return mlfn;
}

void mlf_free_mlfn( mlf_ntup_t *mlfn ) {
  free( mlfn->ntup );
  free( mlfn );
}

void mlf_set_ntup( mlf_def_t *mlf, mlf_ntup_t *mlfn ) {
  int level;

  if ( mlfn->mlf != mlf )
	nl_error( 4, "mlf_set_ntup: Invalid n-tuple" );
  if ( mlfn->suffix != NULL )
	mlf->fsuffix = mlfn->suffix;
  for ( level = 0; level <= mlf->n_levels; level++ )
	mlf->flvl[level].index = mlfn->ntup[level];
  if ( mlfn->base != NULL ) {
	int end = sprintf( mlf->fpath, "%s", mlfn->base );
	mlf->flvl[0].s = mlf->fpath + end;
  }
  { int end, i;
  
	for ( i = 0; i <= mlf->n_levels; i++ ) {
	  if ( mlf->flags & MLF_WRITING ) {
		struct stat buf;
		if ( stat( mlf->fpath, &buf ) || ! S_ISDIR(buf.st_mode) ) {
		  if ( mkdir( mlf->fpath, 0775 ) != 0 )
			nl_error( 3, "Unable to create directory %s", mlf->fpath );
		}
	  }
	  if ( i >= mlf->n_levels ) break;
	  end = sprintf( mlf->flvl[i].s, "/%02d", mlf->flvl[i].index );
	  mlf->flvl[i+1].s = mlf->flvl[i].s + end;
	}
  }
}

/* Return 0 on success, 1 on any syntax error */

mlf_def_t *mlf_init( int n_levels, int n_files, int writing,
	char *fbase, char *fsuffix, char *config ) {
  mlf_def_t *mlf;
  mlf_ntup_t *mlfn;

  if ( n_levels < 1 )
	nl_error( 3, "mlf_init: n_levels must be >= 1" );
  if ( n_files < 2 )
	nl_error( 3, "mlf_init: n_files must be >= 2" );
  n_levels--;
  mlf = new_memory( sizeof(mlf_def_t) );
  mlf->flvl = new_memory( (n_levels+1) * sizeof(mlf_elt_t) );
  mlf->n_levels = n_levels;
  mlf->n_files = n_files;
  mlf->flags = writing ? MLF_WRITING : 0;
  { int end = sprintf( mlf->fpath, "%s", "DATA" );
	mlf->flvl[0].s = mlf->fpath+end;
  }
  mlf->fsuffix = (fsuffix == NULL) ? "log" : nl_strdup(fsuffix);
  mlfn = mlf_convert_fname( mlf, fbase, config );
  mlf_set_ntup( mlf, mlfn );
  mlf_free_mlfn( mlfn );
  return mlf;
}

/* returns < 0 if current file position preceeds mlfn,
   0 if equal, >0 if later
*/
int mlf_compare( mlf_def_t *mlf, mlf_ntup_t *mlfn ) {
  int i, diff = 0;
  for ( i = 0; diff == 0 && i <= mlf->n_levels; i++ )
	diff = mlf->flvl[i].index - mlfn->ntup[i];
  if ( diff == 0 && ( mlf->flags & MLF_INC_FIRST ) == 0 )
	diff = -1;
  return diff;
}

static next_file( mlf_def_t *mlf, int level ) {
  if ( mlf->flags & MLF_INC_FIRST ) {
	if ( ++mlf->flvl[level].index >= mlf->n_files && level > 0 ) {
	  mlf->flvl[level].index = 0;
	  next_file( mlf, level-1 );
	}
  } else mlf->flags |= MLF_INC_FIRST;
  if ( level < mlf->n_levels ) {
	int n;
	
	n = sprintf( mlf->flvl[level].s, "/%02d", mlf->flvl[level].index );
	mlf->flvl[level+1].s = mlf->flvl[level].s + n;
	if ( mlf->flags & MLF_WRITING && mkdir( mlf->fpath, 0775 ) != 0 )
	  nl_error( 2, "Unable to create directory %s", mlf->fpath );
  } else {
	sprintf( mlf->flvl[level].s, "/%02d.%s", mlf->flvl[level].index,
		mlf->fsuffix );
  }
}

FILE *mlf_next_file( mlf_def_t *mlf ) {
  FILE *fp;
  
  next_file( mlf, mlf->n_levels );
  fp = fopen( mlf->fpath, (mlf->flags & MLF_WRITING) ? "w" : "r" );
  return fp;
}
