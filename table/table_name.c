#include "csm.h"

/* get_table_name() strips the extension from the input file name.
   Returns an index into the string table
 */
int get_table_name(void) {
  char *fname;
  int i;
  if ( InputFile == NoKey ) return NoStr;
  fname = StringTable(GetClpValue(InputFile,0));
  for ( i = 0; fname[i] != '\0' && fname[i] != '.'; i++);
  return StoStr(fname,i);
}
