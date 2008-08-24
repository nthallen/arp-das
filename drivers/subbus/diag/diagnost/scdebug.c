/* scdebug.c system controller low-level debug
 * Must be linked privity=1, run as root
*/
#pragma off (unreferenced)
  static char *rcsid =
	"$Id$";
#pragma on (unreferenced)
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "diaglib.h"

#ifdef __USAGE
%C
	System Controller low-level diagnostic
#endif

#define SC_SB_RESET 0x310
#define SC_SB_PORTA 0x308
#define SC_SB_PORTB 0x30A
#define SC_SB_PORTC 0x30C
#define SC_SB_CONTROL 0x30E
#define SC_SB_CONFIG 0xC1C0
#define SC_CMDENBL 0x318
#define SC_NVADDR 0x31A
#define SC_NVRAM 0x31D
#define SC_TICK 0x319
#define SC_DISARM 0x311
#define SC_INPUT 0x31C
#define SC_NMIENBL 0x31C
#define SC_LOWPOWER (inp(SC_SB_PORTC+1)&4)
#define SC_CMDENBL_BCKPLN (inp(SC_SB_PORTC+1)&2)

unsigned short patterns[] = {0, 0xFFFF, 0x00FF, 0x0055, 0xFE01, 0xFD02,
		 0xFB04, 0xF708, 0xEF10, 0xDF20, 0xBF40, 0x7F80};
#define N_PATTERNS (sizeof(patterns)/sizeof(unsigned short))
int n_writes = 1;

int wait_time( void ) {
  int i;
  for (i = 0; i < 10; i++ ) {
	if ( ( inpw( SC_SB_PORTC ) & 0x800 ) == 0 )
	  return 1;
  }
  return 0;
}

/* iorw2 performs the actual out and in */
unsigned short iorw2(unsigned short addr, unsigned short odata, int word) {
  unsigned short idata;
  int i;
  
  if (word) {
	outpw(addr, odata);
	if ( wait_time() ) idata = inpw(addr);
	else idata = ~odata;
  } else {
	outp(addr, odata);
	/* idata = inp(addr) & 0xFF; */
	if ( wait_time() ) idata = inp(addr);
	else idata = ~odata;
  }
  return(idata);
}

/* iorw1 loops if there is an error */
void iorw1(unsigned address, unsigned odata, int word) {
  unsigned short int idata;

  idata = iorw2(address, odata, word);
  if (idata != odata) {
	char *pat = word ? "Wrote %04X Read %04X: diff %04X %04X\r" :
	  ((address&1) ? "Wrote %02X   Read %02X  : diff %02X   %02X\r" :
					"Wrote   %02X Read   %02X: diff   %02X   %02X\r" );
	
	while (kbhit()) getch();
	while (!kbhit()) {
	  if (idata != odata) {
		unsigned diff = idata ^ odata;
		unsigned bits = idata & diff;
		printf("Error at 0x%3X: ", address);
		printf(pat, odata, idata, diff, bits );
		fflush(stdout);
	  }
	  if (dl_skip_checks) break;
	  /* idata = iorw2(address, ~odata, word); */
	  idata = iorw2(address, odata, word);
	}
	putchar('\n'); while (kbhit()) if (getch() == '\033') dl_skip_checks = 1;
  }
}

/* iorw decides the basic behaviour depending on word or byte */
void iorw(unsigned address, int word) {
  int i;

  for (i = 0; i < N_PATTERNS; i++) {
	if (word) iorw1(address, patterns[i], 1);
	else {
	  iorw1(address, patterns[i] & 0xFF, 0);
	  iorw1(address, (patterns[i] >> 8) & 0xFF, 0);
	}
  }
}

int main( int argc, char **argv ) {
  int ver;
  
  if ( argc > 1 ) {
    int nw = atoi(argv[1]);
	if ( nw > 1 ) {
	  printf( "Setting n_writes to %d\n", nw );
	  n_writes = nw;
	}
  }
  /* initialize the subbus */
  outp(SC_SB_RESET, 0);
  outpw(SC_SB_CONTROL, SC_SB_CONFIG);
  ver = inpw( 0x30A );
  if ( ver != 0x4B01 ) {
	printf("Version Error: Read %04X instead of 4B01\n",
			ver);
	fflush(stdout);
  }
    
  while ( dl_skip_checks == 0 ) {
	/* verify the low byte */
	iorw(SC_SB_PORTA, 0);
  
	/* verify the high byte */
	iorw(SC_SB_PORTA+1, 0);
  
	/* verify word writing */
	iorw(SC_SB_PORTA, 1);

	while (kbhit()) {
	  if ( getch() == '\033' ) dl_skip_checks = 1;
	}
  }
  while (kbhit()) getch();
  return(0);
}
