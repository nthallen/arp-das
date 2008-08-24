/* checkrw.c diagnostic library routine.
 * $Log$
 * Revision 1.1  2008/07/23 13:05:18  nort
 * Imported
 *
 */
#include <stdio.h>
#ifdef __QNX__
  #include <conio.h>
#else
  #include <dos.h>
#endif
#include "subbus.h"
#include "diaglib.h"

#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

extern int dl_skip_checks;

static unsigned short patterns[] = {
  0x5555, 0xAAAA, 0x5A5A, 0xA5A5, 0x00FF, 0xFF00, 0xFFFF, 0};
#define N_PATTERNS (sizeof(patterns)/sizeof(unsigned short))

void check_rw(unsigned short address, unsigned short omask) {
  unsigned short int i, odata, idata;
  for (i = 0; i < N_PATTERNS; i++) {
    odata = patterns[i] & omask;
    write_subbus(0, address, odata);
    idata = read_subbus(0, address) & omask;
    if (idata != odata) {
      printf("Error reading back from address %X\n", address);
      fflush(stdout); while (kbhit()) getch();
      while (!kbhit()) {
		printf("Wrote %04X Read %04X (mask %04X)\r", odata, idata, omask);
		if (dl_skip_checks) break;
		write_subbus(0, address, odata);
		idata = read_subbus(0, address) & omask;
      }
      putchar('\n'); while (kbhit()) if (getch() == '\033') dl_skip_checks = 1;
    }
  }
}
