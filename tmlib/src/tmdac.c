#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "tm.h"

static FILE *open_path( char *path, char *fname ) {
  char filename[PATH_MAX];
  if ( snprintf( filename, PATH_MAX, "%s/%s", path, fname )
          >= PATH_MAX )
    nl_error( 3, "Pathname overflow for file '%s'", fname );
  FILE *tmd = fopen( filename, "r" );
  return tmd;
}

void load_tmdac( char *path ) {
  if ( path == NULL || path[0] == '\0' ) path = ".";
  FILE *dacfile = open_path( path, "tm.dac" );
  if ( dacfile == NULL ) {
    char version[40];
    char dacpath[80];

    version[0] = '\0';
    FILE *ver = open_path( path, "VERSION" );
    if ( ver != NULL ) {
      int len;
      if ( fgets( version, 40, ver ) == NULL )
        nl_error(3,"Error reading VERSION: %s",
          strerror(errno));
      len = strlen(version);
      while ( len > 0 && isspace(version[len-1]) )
        version[--len] = '\0';
      if ( len == 0 )
        nl_error( 1, "VERSION was empty: assuming 1.0" );
    } else {
      nl_error( 1, "VERSION not found: assuming 1.0" );
    }
    if ( version[0] == '\0' ) strcpy( version, "1.0" );
    snprintf( dacpath, 80, "bin/%s/tm.dac" );
    dacfile = open_path( path, dacpath );
  }
  if ( dacfile == NULL ) nl_error( 3, "Unable to locate tm.dac" );
  if ( fread(&tm_info.tm, sizeof(tm_dac_t), 1, dacfile) != 1 )
    nl_error( 3, "Error reading tm.dac" );
  fclose(dacfile);
}
