/* failtest.c
 * $Log$
 * Revision 1.1  2008/07/23 13:07:49  nort
 * Uncommitted changes and imports
 *
 *
 */
#include <stdlib.h>
#include <conio.h>
char failtest_c_rcsid[] = "$Id$";

#ifdef __USAGE
%C	n
	Where n is one of:
	0 - Turn off both fail indicators
	1 - Turn on just operate fail indicator
	2 - Turn on just record fail indicator
	3 - Turn on both fail indicators
#endif

int main(int argc, char **argv) {
  int mask;

  if (argc < 2) print_usage(argv);
  else {
	mask = atoi(argv[1]);
	outp(0x319, 0); /* tick to turn of 2-minute failure */
	outp(0x311, 0);
	outp(0x317, mask & 3);
  }
  return(0);
}
