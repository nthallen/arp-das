/* da_cache.c */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "subbusd_int.h"
#include "oui.h"
#include "nortlib.h"

/* *_max_addr is outside the range. This is different
   from the command-line option. The adjustment is made
   in main() after oui_init_options();
*/
static unsigned short hw_min_addr, hw_max_addr;
static unsigned short sw_min_addr, sw_max_addr;

static unsigned short *hwcache, *swcache;

char *cache_hw_range, *cache_sw_range;

/* return:
   1 if HW cache (meaning caller should write to HW)
   0 if SW cache (meaning done)
   -1 if invalid address
 */
int sb_cache_write( unsigned short addr, unsigned short data ) {
  int cache_addr;
  if ( addr < hw_max_addr &&
      addr >= hw_min_addr ) {
    cache_addr = ( addr - hw_min_addr ) / 2;
    hwcache[cache_addr] = data;
    nl_error( -3, "Write to HW Cache: %04X -> [%03X]",
      data, addr );
    return 1;
  } else if ( addr < sw_max_addr &&
              addr >= sw_min_addr ) {
    cache_addr = addr - sw_min_addr;
    swcache[cache_addr] = data;
    nl_error( -3, "Write to SW Cache: %d -> [%03X]",
      data, addr );
    return 0;
  } else {
    return -1;
  }
}

/* return:
   1 if HW cache (meaning done)
   0 if SW cache (meaning done)
   -1 if invalid address
 */
int sb_cache_read( unsigned short addr, unsigned short *data ) {
  int cache_addr;
  if ( addr < hw_max_addr &&
      addr >= hw_min_addr ) {
    cache_addr = ( addr - hw_min_addr ) / 2;
    *data = hwcache[cache_addr];
    nl_error( -3, "Read from HW Cache: [%03X] -> 0x%04X",
      addr, *data );
    return 1;
  } else if ( addr < sw_max_addr &&
              addr >= sw_min_addr ) {
    cache_addr = addr - sw_min_addr;
    *data = swcache[cache_addr];
    nl_error( -3, "Read from SW Cache: %d <- [%03X]",
      *data, addr );
    return 0;
  } else {
    return -1;
  }
}

static void process_range( char *txt,
  unsigned short *min, unsigned short *max ) {
  if ( txt == NULL ) {
    *min = *max = 0;
  } else {
    char *s, *t;
    s = t = txt;
    if ( !isxdigit(*t) )
      nl_error( 3, "Expected hex digit at start of arg \"%s\"",
        txt );
    while ( isxdigit(*t) ) t++;
    if ( *t++ != '-' )
      nl_error( 3, "Expected '-' in arg \"%s\"", txt );
    *min = atoh(s);
    s = t;
    if ( !isxdigit(*t) )
      nl_error( 3, "Expected hex digit after '-': \"%s\"",
        txt );
    while ( isxdigit(*t) ) t++;
    if ( *t != '\0' )
      nl_error( 3, "Garbage after range: \"%s\"", txt );
    *max = atoh(s);
    if ( *min > *max )
      nl_error( 3, "Invalid range: \"%s\"", txt );
    if ( *max > 0xFFFD )
      nl_error( 3, "Range max too high: \"%s\"", txt );
  }
}

void sb_cache_init(void) {
  process_range( cache_hw_range, &hw_min_addr, &hw_max_addr );
  if ( cache_hw_range )	hw_max_addr = (hw_max_addr + 2) & ~1;

  process_range( cache_sw_range, &sw_min_addr, &sw_max_addr );
  if ( cache_sw_range ) sw_max_addr++;
  
  if ( sw_min_addr < hw_max_addr )
    nl_error( 3, "-S range must be above -H range" );
  
  if ( hw_max_addr > hw_min_addr ) {
    int n = (hw_max_addr - hw_min_addr)/2;
    hwcache = new_memory(n*sizeof(unsigned short));
    memset( hwcache, '\0', n*sizeof(unsigned short));
    nl_error( -2,
      "Established %d words of hardware Cache %03X-%03X",
      n, hw_min_addr, hw_max_addr );
  }
  if ( sw_max_addr > sw_min_addr ) {
    int n = (sw_max_addr - sw_min_addr);
    swcache = new_memory(n*sizeof(unsigned short));
    memset( swcache, '\0', n*sizeof(unsigned short));
    nl_error( -2,
      "Established %d words of software Cache %03X-%03X",
      n, sw_min_addr, sw_max_addr );
  }
}
