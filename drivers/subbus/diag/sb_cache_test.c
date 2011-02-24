#include "subbus.h"
#include "nortlib.h"

int main( int argc, char **argv ) {
  unsigned short data;

  if ( !load_subbus() )
    nl_error( 3, "Unable to locate subbus" );
  if ( !cache_write( 0xC60, 0x5555 ) )
    nl_error( 2, "No ack writing to cache at 0xC60" );
  data = cache_read( 0xC60 );
  if ( data != 0x5555 )
    nl_error( 2, "cache_read(0xC60) returned 0x%04d, expected 0x5555",
      data );
  return 0;
}
