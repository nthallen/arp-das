%{
  #ifdef SERVER
    
  void write_savelog( const char *s ) {
    FILE *fp;
    fp = fopen( "saverun.log", "a" );
    if ( fp == 0 ) nl_error( 2, "Unable to write to saverun.log" );
    else {
      fprintf( fp, "%s\n", s );
      fclose( fp );
    }
  }
  
  #endif /* SERVER */
%}

&command
  : SaveLog %s (Enter Log Message) * { write_savelog( $2 ); }
  ;
