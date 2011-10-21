/* digital2.c is a diagnostic for checking out any and all digital
 * input or output boards in their flight configurations.
 * $Log$
 * Revision 1.2  2007/11/21 16:50:39  nort
 * Source
 *
 * Revision 1.1  1996/02/15  19:48:07  eil
 * Initial revision
 *
 * Revision 1.4  1995/06/13  01:16:16  nort
 * Added support for Syscon104 diagnostic features
 * Eliminated Obsolete DOS compatibility
 * Still need to fix up chip labels for different revs.
 *
 * Revision 1.3  1992/10/25  02:00:57  nort
 * Added Water Vapor
 *
 * Revision 1.2  1992/10/25  01:57:31  nort
 * Same as previous
 *
 * Revision 1.2  1992/07/23  19:14:46  nort
 * Mods for QNX4
 *
 * Revision 1.1  1992/07/22  14:24:15  nort
 * Initial revision
 *
 * Modified from digital to incorporate everything. Dec. 23, 1991
 * Modified from digio2.c July 22, 1990
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "subbus.h"

#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

/* The test will not require the user to select which board s/he wishes
   to test.  The diagnostic knows which ports are input and which
   are output.  All output ports are initialized to output high
   levels.  When an output port is selected, the program will toggle
   individual data lines, allowing the tester to check the output
   with an oscilloscope or volt meter.

   The test will first attempt to address every board. If the board
   acknowledges, it will be presumed present (unless permanent ack
   is detected). On all present boards, patterns will be written
   to (all, some) output ports and read back. Any problems will
   be reported.
  
   PgDn		Advance to the next port
   PgUp		Return to the previous port
   Right Arrow	Advance to the next bit (ignored on input ports)
   Left Arrow	Return to the previous bit (ignored on input ports)
   Space	Single step toggles
   Enter	Toggle at will (default)
   Esc		Exit diagnostic
   C		Toggle cmdenbl/ (asserted by default)
   S		Toggle cmdstrobe
   I            Set Programmable Port to Input Mode
   O            Set Programmable Port to Output Mode
   R            Reset current Digio64 connector to default configuration
   E            Switch cmdenbl/cmdstrobe distribution thru current Connector
   X            PORTS READBACK TEST: ===DISCONNECT DIGIO CONNECTORS===
*/
#ifdef __QNX__
  #define EXTENDED_KEY 0xFF
  #define	KEY_PGUP	0x01A2
  #define	KEY_PGDN	0x01AA
  #define	KEY_LEFT	0x01A4
  #define	KEY_RIGHT	0x01A6
  #define	KEY_DOWN	0x01A9
  #define	KEY_UP		0x01A1
  #define	KEY_F1		0x0181
#else
  #define EXTENDED_KEY 0
  #define	KEY_PGUP	0x0149
  #define	KEY_PGDN	0x0151
  #define	KEY_LEFT	0x014b
  #define	KEY_RIGHT	0x014d
  #define	KEY_DOWN	0x0150
  #define	KEY_UP		0x0148
  #define	KEY_F1		0x013b
#endif

char *help_text[] = {
  "   PgDn\t\t\tAdvance to the next Port",
  "   PgUp\t\t\tReturn to the previous Port",
  "   Right Arrow\tAdvance to the next bit of Output Port",
  "   Left Arrow\tReturn to the previous bit of Output Port",
  "   Space\t\tSingle step toggles",
  "   Enter\t\tToggle at will (default)",
  "   I\t\t\tSet Programmable port to Input Mode",
  "   O\t\t\tSet Programmable port to Output Mode",
  "   Esc\t\t\tExit diagnostic",
  "   C\t\t\tToggle cmdenbl/ (asserted by default)",
  "   S\t\t\tToggle cmdstrobe",
  "   R\t\t\tReset a Digio64 50-pin Connector to Default Configuration",
  "   E\t\t\tSwitch cmdenable/cmdstrobe distribution thru current Connector",
  "   X\t\t\tPORTS READBACK TEST: ===DISCONNECT DIGIO CONNECTORS===",
  "   H or F1\t\tDisplay this message",
  NULL
};

int cmdenbl = 0;

void help(void) {
  char **p;
  putchar('\n');
  for (p = help_text; *p != NULL; p++) printf("%s\n", *p);
}

/* Terminates with an error message. Could be expanded to take
   additional arguments if necessary.
*/
void app_error(char *text) {
  fprintf(stderr, "%s\n", text);
  exit(1);
}

/* The following information is needed for each port:
    Input or Output?
    Are the connector pins in order? (FL_SHUFFLED)
    Address (high or low byte is implicit)
    Which chip and port
    Which connector and pins
    What order are the bits on the buffer?
*/
#define FL_PROGRAMMABLE 0
#define FL_OUTPUT 1
#define FL_INPUT 4
#define FL_SHUFFLED 2

#define HERE_NOT_ICC 0
#define HERE_ICC 1
#define HERE_104 2
#define HERE_NOT_104 -1

struct bddf {
  int present;
  char *name;
  unsigned int control;
  unsigned int initcmd;
} boards[] = {
  HERE_NOT_ICC, "Disc0", 0x806, 0x8989,
  HERE_NOT_ICC, "Disc1", 0x826, 0x8989,
  HERE_NOT_ICC, "Disc2", 0x846, 0x8989,
  HERE_NOT_ICC, "Dstat", 0x406, 0x9B9B,
  HERE_NOT_ICC, "Digio0 Chips 1&3", 0x866, 0x8282,
  HERE_NOT_ICC, "Digio0 Chips 2&4", 0x886, 0x8989,
  HERE_NOT_ICC, "Digio1 Chips 1&3", 0x8A6, 0x8282,
  HERE_NOT_ICC, "Digio1 Chips 2&4", 0x8C6, 0x8989,
  HERE_NOT_ICC, "Main Ozone", 0x8E6, 0x0089,
  HERE_NOT_ICC, "Water Vapor", 0x8EE, 0x0089,
  HERE_NOT_104, "Digio64 50pin Connector @ Addr 0x800", 0x806, 0xFFFF,
  HERE_NOT_104, "Digio64 50pin Connector @ Addr 0x820", 0x826, 0xFFFF,
  HERE_NOT_104, "Digio64 50pin Connector @ Addr 0x840", 0x846, 0xFFFF,
  HERE_NOT_104, "Digio64 50pin Connector @ Addr 0x860", 0x866, 0xFFFF,
  HERE_NOT_104, "Digio64 50pin Connector @ Addr 0x880", 0x886, 0xFFFF,
  HERE_NOT_104, "Digio64 50pin Connector @ Addr 0x8A0", 0x8A6, 0xFFFF,
  HERE_NOT_104, "Digio64 50pin Connector @ Addr 0x8C0", 0x8C6, 0xFFFF,
  HERE_NOT_104, "Digio64 50pin Connector @ Addr 0x8E0", 0x8E6, 0xFFFF
};
#define N_BOARDS (sizeof(boards)/sizeof(struct bddf))

unsigned int pattern[] = { 0, 0x5555, 0xAAAA, 0xFFFF, 0x0101, 0x0202,
  0x0404, 0x0808, 0x1010, 0x2020, 0x4040, 0x8080 };
#define N_PATS (sizeof(pattern)/sizeof(unsigned int))

/* orders defines the order of bits on the buffer chips */
int orders[5][8] = {
  9,7,5,3,2,4,6,8,
  7,8,9,6,5,4,3,2,
  9,8,7,6,5,4,3,2,
  3,4,5,6,7,8,9,2,
  2,3,4,5,6,7,8,9
};

struct pd {
  int board;
  char *chip;
  char port;
  unsigned int address;
  char *bufchip;
  int bufpinorder;
  char *con_name;
  int con_pin;
  int flags;
} port_data[] = {
  0, "Z9", 'A', 0x808, "Z11", 0, "J8", 2, FL_OUTPUT,
  0, "Z9", 'B', 0x80A, "Z12", 1, "J8", 10, FL_OUTPUT,
  0, "Z10", 'A', 0x811, "Z14", 0, "J8", 21, FL_OUTPUT,
  0, "Z10", 'B', 0x813, "Z13", 2, "J8", 29, FL_OUTPUT,
  1, "Z9", 'A', 0x828, "Z11", 0, "J8", 2, FL_OUTPUT,
  1, "Z9", 'B', 0x82A, "Z12", 1, "J8", 10, FL_OUTPUT,
  1, "Z10", 'A', 0x831, "Z14", 0, "J8", 21, FL_OUTPUT,
  1, "Z10", 'B', 0x833, "Z13", 2, "J8", 29, FL_OUTPUT,
  2, "Z9", 'A', 0x848, "Z11", 0, "J8", 2, FL_OUTPUT,
  2, "Z9", 'B', 0x84A, "Z12", 1, "J8", 10, FL_OUTPUT,
  2, "Z10", 'A', 0x851, "Z14", 0, "J8", 21, FL_OUTPUT,
  2, "Z10", 'B', 0x853, "Z13", 2, "J8", 29, FL_OUTPUT,
  3, "Z9", 'A', 0x408, "Z11", 0, "J8", 2, FL_INPUT,
  3, "Z9", 'B', 0x40A, "Z12", 1, "J8", 10, FL_INPUT,
  3, "Z10", 'A', 0x411, "Z14", 0, "J8", 21, FL_INPUT,
  3, "Z10", 'B', 0x413, "Z13", 2, "J8", 29, FL_INPUT,
  4, "U1", 'A', 0x868, "U11", 2, "J8",  2, FL_OUTPUT,
  4, "U1", 'B', 0x86A, "U12", 2, "J8", 10, FL_INPUT,
  4, "U1", 'C', 0x86C, "U13", 3, "J8", 18, FL_OUTPUT | FL_SHUFFLED,
  5, "U2", 'A', 0x888, "U15", 2, "J8", 26, FL_OUTPUT,
  5, "U2", 'B', 0x88A, "U16", 2, "J8", 34, FL_OUTPUT,
  5, "U2", 'C', 0x88C, "U17", 3, "J8", 42, FL_INPUT | FL_SHUFFLED,
  4, "U3", 'A', 0x871, "U19", 2, "J9",  2, FL_OUTPUT,
  4, "U3", 'B', 0x873, "U20", 2, "J9", 10, FL_INPUT,
  4, "U3", 'C', 0x875, "U21", 3, "J9", 18, FL_OUTPUT | FL_SHUFFLED,
  5, "U4", 'A', 0x891, "U23", 2, "J9", 26, FL_OUTPUT,
  5, "U4", 'B', 0x893, "U24", 2, "J9", 34, FL_OUTPUT,
  5, "U4", 'C', 0x895, "U25", 3, "J9", 42, FL_INPUT | FL_SHUFFLED,
  6, "U1", 'A', 0x8A8, "U11", 2, "J8",  2, FL_OUTPUT,
  6, "U1", 'B', 0x8AA, "U12", 2, "J8", 10, FL_INPUT,
  6, "U1", 'C', 0x8AC, "U13", 3, "J8", 18, FL_OUTPUT | FL_SHUFFLED,
  7, "U2", 'A', 0x8C8, "U15", 2, "J8", 26, FL_OUTPUT,
  7, "U2", 'B', 0x8CA, "U16", 2, "J8", 34, FL_OUTPUT,
  7, "U2", 'C', 0x8CC, "U17", 3, "J8", 42, FL_INPUT | FL_SHUFFLED,
  6, "U3", 'A', 0x8B1, "U19", 2, "J9",  2, FL_OUTPUT,
  6, "U3", 'B', 0x8B3, "U20", 2, "J9", 10, FL_INPUT,
  6, "U3", 'C', 0x8B5, "U21", 3, "J9", 18, FL_OUTPUT | FL_SHUFFLED,
  7, "U4", 'A', 0x8D1, "U23", 2, "J9", 26, FL_OUTPUT,
  7, "U4", 'B', 0x8D3, "U24", 2, "J9", 34, FL_OUTPUT,
  7, "U4", 'C', 0x8D5, "U25", 3, "J9", 42, FL_INPUT | FL_SHUFFLED,
  8, "U20", 'A', 0x8E0, "U23", 4, "VC2", 2, FL_OUTPUT,
  8, "U20", 'B', 0x8E2, "U24", 4, "VC2", 18, FL_OUTPUT,
  8, "U20", 'C', 0x8E4, "U25", 4, "VC2", 10, FL_INPUT,
  9, "U20", 'A', 0x8E8, "U23", 4, "VC2", 2, FL_OUTPUT,
  9, "U20", 'B', 0x8EA, "U24", 4, "VC2", 18, FL_OUTPUT,
  9, "U20", 'C', 0x8EC, "U25", 4, "VC2", 10, FL_INPUT,
  10, "U1", '1', 0x808, "U1", -1, "J", 2, FL_PROGRAMMABLE,
  10, "U1", '2', 0x811, "U1", -1, "J", 10, FL_PROGRAMMABLE,
  10, "U1", '3', 0x80A, "U1", -1, "J", 18, FL_PROGRAMMABLE,
  10, "U1", '4', 0x813, "U1", -1, "J", 26, FL_PROGRAMMABLE,
  10, "U1", '5', 0x80C, "U1", -1, "J", 34, FL_PROGRAMMABLE,
  10, "U1", '6', 0x815, "U1", -1, "J", 42, FL_PROGRAMMABLE,
  11, "U1", '1', 0x828, "U1", -1, "J", 2, FL_PROGRAMMABLE,
  11, "U1", '2', 0x831, "U1", -1, "J", 10, FL_PROGRAMMABLE,
  11, "U1", '3', 0x82A, "U1", -1, "J", 18, FL_PROGRAMMABLE,
  11, "U1", '4', 0x833, "U1", -1, "J", 26, FL_PROGRAMMABLE,
  11, "U1", '5', 0x82C, "U1", -1, "J", 34, FL_PROGRAMMABLE,
  11, "U1", '6', 0x835, "U1", -1, "J", 42, FL_PROGRAMMABLE,
  12, "U1", '1', 0x848, "U1", -1, "J", 2, FL_PROGRAMMABLE,
  12, "U1", '2', 0x851, "U1", -1, "J", 10, FL_PROGRAMMABLE,
  12, "U1", '3', 0x84A, "U1", -1, "J", 18, FL_PROGRAMMABLE,
  12, "U1", '4', 0x853, "U1", -1, "J", 26, FL_PROGRAMMABLE,
  12, "U1", '5', 0x84C, "U1", -1, "J", 34, FL_PROGRAMMABLE,
  12, "U1", '6', 0x855, "U1", -1, "J", 42, FL_PROGRAMMABLE,
  13, "U1", '1', 0x868, "U1", -1, "J", 2, FL_PROGRAMMABLE,
  13, "U1", '2', 0x871, "U1", -1, "J", 10, FL_PROGRAMMABLE,
  13, "U1", '3', 0x86A, "U1", -1, "J", 18, FL_PROGRAMMABLE,
  13, "U1", '4', 0x873, "U1", -1, "J", 26, FL_PROGRAMMABLE,
  13, "U1", '5', 0x86C, "U1", -1, "J", 34, FL_PROGRAMMABLE,
  13, "U1", '6', 0x875, "U1", -1, "J", 42, FL_PROGRAMMABLE,
  14, "U1", '1', 0x888, "U1", -1, "J", 2, FL_PROGRAMMABLE,
  14, "U1", '2', 0x891, "U1", -1, "J", 10, FL_PROGRAMMABLE,
  14, "U1", '3', 0x88A, "U1", -1, "J", 18, FL_PROGRAMMABLE,
  14, "U1", '4', 0x893, "U1", -1, "J", 26, FL_PROGRAMMABLE,
  14, "U1", '5', 0x88C, "U1", -1, "J", 34, FL_PROGRAMMABLE,
  14, "U1", '6', 0x895, "U1", -1, "J", 42, FL_PROGRAMMABLE,
  15, "U1", '1', 0x8A8, "U1", -1, "J", 2, FL_PROGRAMMABLE,
  15, "U1", '2', 0x8B1, "U1", -1, "J", 10, FL_PROGRAMMABLE,
  15, "U1", '3', 0x8AA, "U1", -1, "J", 18, FL_PROGRAMMABLE,
  15, "U1", '4', 0x8B3, "U1", -1, "J", 26, FL_PROGRAMMABLE,
  15, "U1", '5', 0x8AC, "U1", -1, "J", 34, FL_PROGRAMMABLE,
  15, "U1", '6', 0x8B5, "U1", -1, "J", 42, FL_PROGRAMMABLE,
  16, "U1", '1', 0x8C8, "U1", -1, "J", 2, FL_PROGRAMMABLE,
  16, "U1", '2', 0x8D1, "U1", -1, "J", 10, FL_PROGRAMMABLE,
  16, "U1", '3', 0x8CA, "U1", -1, "J", 18, FL_PROGRAMMABLE,
  16, "U1", '4', 0x8D3, "U1", -1, "J", 26, FL_PROGRAMMABLE,
  16, "U1", '5', 0x8CC, "U1", -1, "J", 34, FL_PROGRAMMABLE,
  16, "U1", '6', 0x8D5, "U1", -1, "J", 42, FL_PROGRAMMABLE,
  17, "U1", '1', 0x8E8, "U1", -1, "J", 2, FL_PROGRAMMABLE,
  17, "U1", '2', 0x8F1, "U1", -1, "J", 10, FL_PROGRAMMABLE,
  17, "U1", '3', 0x8EA, "U1", -1, "J", 18, FL_PROGRAMMABLE,
  17, "U1", '4', 0x8F3, "U1", -1, "J", 26, FL_PROGRAMMABLE,
  17, "U1", '5', 0x8EC, "U1", -1, "J", 34, FL_PROGRAMMABLE,
  17, "U1", '6', 0x8F5, "U1", -1, "J", 42, FL_PROGRAMMABLE
};
#define N_PORTS (sizeof(port_data)/sizeof(struct pd))

/* state definitions for the state variable */
#define STATE_EXIT 0
#define STATE_NEW_PORT 1
#define STATE_NEW_BIT 2
#define STATE_LOOPING 3
/* state definitions for the looping variable */
#define LOOP_WILDLY 0
#define LOOP_ONCE 1
#define LOOP_HOLD 2

int kgetch(void) {
  int c;
  if (kbhit()) {
    c = getch();
    if (c == EXTENDED_KEY) c = 0x100 | getch();
  } else c = -1;
  return c;
}

#define do_reset(ADDR) write_ack(0, ADDR, 0)

/* Digio64 board functions */
#define IS_DIGIO64(B) (boards[B].present == HERE_104)
#define MAKE_DIGIO64(B) (B.present = HERE_104)

void toggle_cmden(unsigned int addr) {
  unsigned int c;
  c = read_subbus(0, addr);
  if (c & 0x1) {
    c &= 0x0;
    printf("Command Enable is distributed through this Connector unit\n");
  }
  else {
    c |= 0x1;
    printf("Command Strobe is distributed through this Connector unit\n");
  }
  write_subbus(0, addr, c);
}

void setport(struct pd *p_d, int in) {
  unsigned int cp;
  unsigned int s = 0;
  /* get control word for board */
  cp = read_subbus(0, boards[p_d->board].control);
  /* set port */
  switch (p_d->port) {
  case '1':
  case '2': s = 0x10; break;
  case '3':
  case '4': s = 0x02; break;
  case '5':
  case '6': s = 0x01; break;
  }
  s = s << ((p_d->address & 1) ? 8 : 0); 
  p_d->flags = FL_PROGRAMMABLE;
  switch(in) {
  case -1:
    if (cp & s) p_d->flags |= FL_INPUT;
    else p_d->flags |= FL_OUTPUT;
    break;
  case 1:
    cp |= s;
    p_d->flags |= FL_INPUT;
    write_subbus(0, boards[p_d->board].control, cp);
    break;
  case 0:
    cp &= ~s;
    p_d->flags |= FL_OUTPUT;
    write_subbus(0, boards[p_d->board].control, cp);
    break;
  }
}

unsigned int read_vers(unsigned int addr) {
  unsigned int r;
  write_subbus(0, 0, 0);
  r = read_subbus(0, addr);
  r &= 0x6060;
  return(r);
}

/* returns non-zero if we know how to set the cmdstrobe */
int do_cmdstrobe( int asserted ) {
  if ( subbus_features & SBF_CMDSTROBE )
	set_cmdstrobe( asserted );
  else if ( subbus_subfunction == SB_SYSCON ) {
	if (asserted) outp(0x30E, 3);
	else outp(0x30E, 2);
  } else return 0;
  return 1;
}

/* Z9-4|----|Z11-9  Z11-11|-------|J8-2 */
void identify_pin(int port, int bit) {
  struct pd *p_d;
  int ppipin, bpin, cpin;

  p_d = &port_data[port];
  if (!IS_DIGIO64(p_d->board)) {
    if (p_d->port == 'A') {
      if (bit < 4) ppipin = 4 - bit;
      else ppipin = 44 - bit;
    } else ppipin = bit + 18;
    printf("%s-P%c%d pin %d <----> ", p_d->chip, p_d->port, bit, ppipin);
    bpin = orders[p_d->bufpinorder][bit];
    printf("%s-%d and %s->%d <----> ",p_d->bufchip,bpin,p_d->bufchip, 20-bpin);
  }
  cpin = p_d->con_pin +
    ((p_d->flags & FL_SHUFFLED) ? (7 - ((bit+1) & 7)) : bit);
  printf("%s-pin %d\n", p_d->con_name, cpin);
}

int new_port(int port, int direction) {
  for (;;) {
    if (direction == 0) direction = 1;
    else if (direction == 1) {
      if (++port == N_PORTS) port = 0;
    } else if (port == 0) port = N_PORTS-1;
    else port--;
    if (boards[port_data[port].board].present==HERE_ICC ||
	boards[port_data[port].board].present==HERE_104) break;
  }
  return(port);
}


void readback (int which) {
  int port, j;
  unsigned int addr, mask, value;

  printf("\nWrite to Ports then Read Back\n");
  for (port = 0; port < N_PORTS; port++) {
    if (boards[port_data[port].board].present == which) {
      if (which==HERE_104) setport(&port_data[port],0);
      if (port_data[port].flags & FL_OUTPUT) {
	addr = port_data[port].address;
	mask = (addr & 1) ? 0xFF00 : 0xFF;
	value = 0;
	for (j = 0; j < N_PATS; j++) {
	  write_subbus(0, addr, pattern[j]);
          write_subbus(0, 0, (unsigned int)(~pattern[j]));
	  if (which==HERE_104)
	    value |= ((~read_subbus(0,addr)) ^ pattern[j]) & mask;
	  else
	    value |= (read_subbus(0,addr) ^ pattern[j]) & mask;
	}
	write_subbus(0, addr, 0);
	if (value != 0)
	  printf("Error: %s chip %s port %c. Bits failing: %04X\n",
		 boards[port_data[port].board].name,
		 port_data[port].chip, port_data[port].port, value);
      }
    }
  }
  printf("Readback Test Completed\n");
}

int do_cmdenable(int set) {
  int old_cmdenbl;
  old_cmdenbl = cmdenbl;
  set_cmdenbl(set);
  cmdenbl = set;
  if (set) printf("\nCMDENBL is asserted\n");
  else printf("\nCMDENBL is not asserted\n");
  return(old_cmdenbl);
}

void main(void) {
  int port, bit, shift, pin, i;
  int state, looping, c, j;
  unsigned int value, mask;
  struct pd *p_d;
  unsigned int pc_104;
  int num_icc_boards = 0, num_104_boards = 0;
  int cmdstrobe = 0;

  printf("Digital Command and Status Diagnostic: " __DATE__ "\n");
  pin = load_subbus();
  if (pin == 0) app_error("Subbus library not resident");
  if (pin != SB_SYSCON && pin != SB_SYSCON104 )
    printf("CMDSTROBE testing will not work properly "
           "without the system controller\n");

  /* Board Detection */
  for (i = 0; i < N_BOARDS; i++)
    /* should only read version after reset or initialisation */
    if (do_reset(boards[i].control+18) != 0)  {
      pc_104 = read_vers(boards[i].control);
      if (pc_104 != 0) {
	if (boards[i].present != HERE_NOT_104) continue;
	else {
	  if (write_ack(0, boards[i].control, boards[i].initcmd) == 0) {
	    printf("Can't initialise board %s",boards[i].name);
	    continue;
	  }
	  MAKE_DIGIO64(boards[i]);
	  num_104_boards++;
	  printf("%s Present\n", boards[i].name);
	}
      } else {
	if (boards[i].present != HERE_NOT_ICC) continue;
	else {
	  if (write_ack(0, boards[i].control, boards[i].initcmd) == 0) {
	    printf("Can't initialise board %s",boards[i].name);
	    continue;
	  }
	  boards[i].present = HERE_ICC;
	  num_icc_boards++;
	  printf("ICC board %s Present\n", boards[i].name);
	}
      }
    }

  if (!(num_icc_boards+num_104_boards)) app_error("No boards present");
  
  /* A reset to any reset address on an ICC board resets all 8255's */
  /* A reset to a 50-pin unit on a Digio64 board resets only that unit */
  /* So, redo the initalisations for Digio ICC boards */
  if (num_icc_boards) {
    for (i = 0; i < N_BOARDS; i++) {
      if (boards[i].present == HERE_ICC)
	write_subbus(0, boards[i].control, boards[i].initcmd);
    }
  }

  printf("Press any key to continue...\n");
  while (kgetch()==-1);

  help();
  looping = LOOP_WILDLY;
  do_cmdenable(1);
  do_cmdstrobe(cmdstrobe);
  port = new_port(0, 0);

  for (state = STATE_NEW_PORT; state == STATE_NEW_PORT;) {
    /* set up for toggling new port/bit */
    p_d = &port_data[port];
    shift = (p_d->address & 1) ? 8 : 0;
    if (IS_DIGIO64(p_d->board)) setport(&port_data[port],-1);
    if (p_d->flags & FL_OUTPUT) {
      bit = 0;
      printf("\nWriting to ");
    } else {
      value = 0x100;
      printf("\nReading from ");
    }

    printf("%s chip %s port %c on connector pins %d-%d\n",
	   boards[p_d->board].name, p_d->chip, p_d->port,
	   p_d->con_pin, p_d->con_pin+7);

    /* Loop for selecting bits */
    for (state = STATE_NEW_BIT; state == STATE_NEW_BIT; ) {
      if (p_d->flags & FL_OUTPUT) {
        value = 0;
	mask = 1 << (shift+bit);
	printf("Toggling ");
	identify_pin(port, bit);
	if (looping == LOOP_HOLD) looping = LOOP_ONCE;
      } else {
	printf(" Bits 0-7 are located on the following pins:\n");
	for (bit = 0; bit < 8; bit++) identify_pin(port, bit);
      }

      /* Here's the tight loop for toggling bits */
      for (state = STATE_LOOPING; state == STATE_LOOPING;) {
	/* first, we'd better tick the SIC periodically */
	tick_sic();
	/* perform the tight loop action */
	/* If output, flip the bit; if input, display the port */
	if ((p_d->flags & FL_OUTPUT) == 0) {
	  mask = (read_subbus(0, p_d->address) >> shift) & 0xFF;
	  if (mask != value) {
	    value = mask;
	    for (mask = 0x80; mask != 0; mask >>= 1)
	      putchar((value & mask) ? '1' : '0');
	    putchar('\r');
		fflush(stdout);
	  }
	} else 
	  if (looping == LOOP_WILDLY || looping == LOOP_ONCE) {
	  value ^= mask;
	  write_subbus(0, p_d->address, value);
	  if (looping == LOOP_ONCE) {
	    if (IS_DIGIO64(p_d->board))
	      printf((value & mask) ? 
  "HIGH (but inverted output = low)\r" : "LOW (but inverted output = high)\r");
	    else
	      printf((value & mask) ? "HIGH     \r" : "LOW      \r");
	    fflush(stdout);
	    looping = LOOP_HOLD;
	  }
	}

	/* Then poll for keyboard input */
	switch (c = kgetch()) {
	  case -1:
	    break;
	  case ' ': /* Single step toggles */
	    looping = LOOP_ONCE;
	    break;
	  case '\r': /* Toggle at will (default) */
	  case '\n':
	    if (p_d->flags & FL_OUTPUT) {
	      printf("TOGGLING\r");
		  fflush(stdout);
		}
	    looping = LOOP_WILDLY;
	    break;
	  case '\033': /* Exit diagnostic */
	    state = STATE_EXIT;
	    break;
	  case 'X':
	  case 'x':
	    /* may want to turn off cmdenable here */
	    if (num_icc_boards) {
	      cmdenbl = do_cmdenable(0);
	      readback(HERE_ICC);
	      do_cmdenable(cmdenbl);
	    }
	    tick_sic();
	    if (num_104_boards) {
	      cmdenbl = do_cmdenable(1);
	      readback(HERE_104);
	      do_cmdenable(cmdenbl);
	    }
	    tick_sic();
	    state=STATE_NEW_PORT;
	    break;
	  case 'C': /* Toggle cmdenbl/ (asserted by default) */
	  case 'c':
	    do_cmdenable(!cmdenbl);
	    break;
	  case 'S': /* Toggle cmdstrobe */
	  case 's':
	    if (do_cmdstrobe(cmdstrobe ^ 1)) {
	      cmdstrobe ^= 1;
	      if (cmdstrobe) printf( "\nCMDSTROBE is asserted\n" );
	      else printf( "\nCMDSTROBE is not asserted\n" );
	    } else printf( "\nDon't know how to set CMDSTROBE\n" );
	    break;
	  case 'R':
	  case 'r':
	    if (IS_DIGIO64(p_d->board)) {
	      do_reset(boards[p_d->board].control + 18);
	      state = STATE_NEW_PORT;	    
	    } else printf("Sorry, this is not a Programmable I/O board\n");
	    break;
	  case 'E':
	  case 'e':
	    if (IS_DIGIO64(p_d->board)) {
	      toggle_cmden(boards[p_d->board].control + 20);	      
	    } else printf("Sorry, this is a Non-Programmable Connector\n");
	    break;
	  case KEY_PGUP: /* Return to the previous port */
	    if (p_d->flags & FL_OUTPUT)
	      write_subbus(0, p_d->address, 0);
	    port = new_port(port, -1);
	    state = STATE_NEW_PORT;
	    break;
	  case KEY_PGDN: /* Advance to the next port */
	    if (p_d->flags & FL_OUTPUT)
	      write_subbus(0, p_d->address, 0);
	    port = new_port(port, 1);
	    state = STATE_NEW_PORT;
	    break;
	  case 'I':
	  case 'i':
	    if (IS_DIGIO64(p_d->board)) {
	      setport(p_d,1);
	      state = STATE_NEW_PORT;
	    }
	    else printf("Sorry, this is a Non-Programmable Port\n");
	    break;
	  case 'O':
	  case 'o':
	    if (IS_DIGIO64(p_d->board)) {
	      setport(p_d,0);
	      state = STATE_NEW_PORT;
	    }
	    else printf("Sorry, this is a Non-Programmable Port\n");
	    break;
	  case KEY_LEFT: /* Return to the previous bit */
	  case KEY_DOWN: /* (ignored on input ports) */
	    if (p_d->flags & FL_OUTPUT) {
	      if (bit == 0) bit = 7;
	      else bit--;
	      state = STATE_NEW_BIT;
	    }
	    break;
	  case KEY_RIGHT: /* Advance to the next bit */
	  case KEY_UP: /* (ignored on input ports) */
	    if (p_d->flags & FL_OUTPUT) {
	      if (bit == 7) bit = 0;
	      else bit++;
	      state = STATE_NEW_BIT;
	    }
	    break;
	  case KEY_F1:
	  case 'H':
	  case 'h':
	    help();
	    break;
	}
      }
    }
  }
  disarm_sic();
  putc('\n', stdout);
}
