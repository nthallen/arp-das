/* checkack.c diagnostic library routine.
 * $Log$
 * Revision 1.1  2008/07/23 13:05:16  nort
 * Imported
 *
 * The following are supposed to be generic diagnostic routines.
 *  The first couple are "datapath" diagnostics.
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

void check_rack(unsigned short address) {
  unsigned short data;

  if (read_ack(0, address, &data) == 0) {
    printf("No acknowledge reading from address %X:", address);
    fflush(stdout); while (kbhit()) getch();
    if (!dl_skip_checks) while (!kbhit()) read_subbus(0, address);
    putchar('\n'); while (kbhit()) if (getch() == '\033') dl_skip_checks = 1;
  }
}

void check_wack(unsigned short address, unsigned short oval) {
  if (write_ack(0, address, oval) == 0) {
    printf("No acknowledge writing to address %X:", address);
    fflush(stdout); while (kbhit()) getch();
    if (!dl_skip_checks) while (!kbhit()) write_subbus(0, address, oval);
    putchar('\n'); while (kbhit()) if (getch() == '\033') dl_skip_checks = 1;
  }
}

void check_ack(unsigned short address, unsigned short oval) {
  check_rack(address);
  check_wack(address, oval);
}

void check_read( unsigned short addr ) {
  if (!dl_skip_checks) {
	printf( "Reading from %04X...", addr );
	fflush(stdout); while (kbhit()) getch();
	while (!kbhit()) read_subbus( 0, addr );
  }
  putchar('\n'); while (kbhit()) if (getch() == '\033') dl_skip_checks = 1;
}

void check_nack( unsigned short addr ) {
  unsigned short data;

  if ( read_ack( 0, addr, &data ) ) {
	printf( "Unexpected acknowledge detected: " );
	check_read( 0 );
  }
}
