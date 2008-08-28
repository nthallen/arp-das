/* BCKPLN.C Interactive testbed for new ICC Backplanes.
 * $Log$
 * Revision 1.1  2008/08/24 15:35:38  ntallen
 * Diagnostics I want to port
 *
 * Revision 1.4  2008/07/23 13:07:43  nort
 * Uncommitted changes and imports
 *
 * Revision 1.3  1995/03/29  14:41:06  nort
 * Separate support for PCICC, Rev. C and Rev. D
 *
 * Revision 1.1  1992/07/23  19:44:25  nort
 * Initial revision
 *
 * Written October 21, 1991
 */
/* Todo 3/29/95:
  fill in sb_read(), sb_write(), sb_cmdenbl() and sb_cmdstrb()
  modify sc104_signals.
*/
#ifdef __QNX__
  #ifdef __QNXNTO__
    #include <sys/neutrino.h>
  #else
    #include <conio.h>
  #endif
#endif
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bckpln.h"
#include "oui.h"

#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int ver=VER_REVD;
int enable_sc104_bufs = 0;
unsigned short SC_SB_LOWCTRL, SC_CMDENBL;
unsigned short SB104_Ctrl = 2;

#ifdef __QNXNTO__
  #include <hw/inout.h>
  #define outword(x,y) out16(x,y)
  #define outbyte(x,y) out8(x,y)
  #define inword(x) in16(x)
  #define inbyte(x) in8(x)
  #define KEY_F1 0x109
  #define KEY_F2 0x10A
  #define KEY_F3 0x10B
  #define KEY_F4 0x10C
  #define KEY_F5 0x10D
  #define KEY_F6 0x10E
  #define KEY_F7 0x10F
  #define KEY_F8 0x110
  #define KEY_F9 0x111
  #define KEY_F10 0x112
  #define KEY_PGUP 0x153
  #define KEY_PGDN 0x152
#else
  #ifdef __QNX__
    #include <i86.h>
    #define outword(x,y) outpw(x,y)
    #define outbyte(x,y) outp(x,y)
    #define inword(x) inpw(x)
    #define inbyte(x) inp(x)
  #else
    #include <os2dev.h>
    void outword(unsigned int addr, unsigned int val);
    void outbyte(unsigned int addr, unsigned int val);
    unsigned int inword(unsigned int addr);
    unsigned int inbyte(unsigned int addr);
  #endif
#endif

void sb_addr( unsigned short x ) {
  if ( ver == VER_PCICC ) {
	outbyte(0x309, (x)&0xFF);
	outbyte(0x30D, (x)>>8);
  } else outword(0x30A, x);
}

void sb_data( unsigned short x ) {
  if ( ver == VER_PCICC ) {
	outbyte(0x308, (x)&0xFF);
	outbyte(0x30C, (x)>>8);
  } else outword(0x308, x);
}

void disable_sic( void ) {
  if ( ver == VER_PCICC ) outbyte(0x318, 0);
  else outbyte(0x311, 0);
}

#define tick_sic() outbyte(0x319, 0)

unsigned int beep_dur = 80;

/*
   The strategy is as follows:
     1. Verify voltages before and after connection of the
        system controller to the backplane.
     2. Interactively test each line over which the computer
        has direct control. This includes ADDRESS, DATA, EXPRD\,
	EXPWR\, CMDENBL\ and CMDSTRB\.
     3. For each line tested:
        A. Test both high and low voltages
	B. Give short audible signal to indicate whether a
	   high or low voltage should be read from the
	   backplane
	C. While testing, toggle all other lines continuously
	   in order to detect and/or measure crosstalk on the
	   backplane or the cable.
	D. Report the chips and pins through which the signal
	   travels on its way to the backplane.
     4. Provide both manual and free-running modes.
	A. Manual mode requires keyhit for each new test, say
	   <space> to toggle the current line, N to advance to
	   the next line, P to go back one ...
	B. Semi-automatic mode requires a keyhit before testing
	   the next line, but will perform (low, high, low, high)
	   automatically.
	C. Free-running mode would be used for rapid checking
	   of the entire address bus or data bus, pausing only
	   briefly between each line.
                     
                            ARP Backplane Diagnostic

            Signal                  Pin                   State
        ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿         ÚÄÄÄÄÄÄÄÄÄÄ¿          ÚÄÄÄÄÄÄÄÄÄÄ¿
        ³   >>>>>>>>   ³         ³   >>>>   ³          ³   >>>>   ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ         ÀÄÄÄÄÄÄÄÄÄÄÙ          ÀÄÄÄÄÄÄÄÄÄÄÙ

    System Controller
   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
   ³ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ÆÍ»
   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ º
            ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ>>ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼  Backplane
            º  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
            ÈÍÍµ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ³
               ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

                    F1  Audio                     >>>
                    F2  Pause before each toggle  >>>
                    F3  Pause before each line    >>>
                    F4  Auto Advance              >>>
              + -       Speed                      >
            <pgup>  F7  Previous Bus
            <pgdn>  F8  Next Bus
            <up>    F9  Previous Signal
            <dn>    F10 Next Signal

 .1. Define fields in terms of row, col, width.
 .2. Use #defines to associate with a particular function.
  4. Provide functions to clear a field and then write a new value
     into the field, perhaps making sure the value will fit.
  5. Check to make sure sc_wire and bp_wire strings are short enough
     to fit into their respective fields.
 .6. Use #defines effectively to make the screen painting adjustable.
*/
#define SIGFLD 0
#define PINFLD 1
#define STAFLD 2
#define SCLIST 3
#define BPLIST 4
#define AUDFLD 5
#define TPSFLD 6
#define LPSFLD 7
#define AAFLD  8
#define SPDFLD 9
#define CBLFLD 10
#define N_FIELDS 11

#define TL_ROW 0
#define SG_ROW 4
#define SCL_ROW 9
#define BPL_ROW 13
#define F1_ROW 16
#define F2_ROW 17
#define F3_ROW 18
#define F4_ROW 19
#define SP_ROW 20
#define F7_ROW 21
#define F8_ROW 22
#define F9_ROW 23
#define F10_ROW 24
#define FT_COL 20
#define HLP_COL (FT_COL-8)
#define TG_COL 50
#define BG_ATTR 7

#define NORM_FREQ 720
#define HIGH_FREQ 440
#define LOW_FREQ 220

#ifdef __QNX__
  #ifdef __QNXNTO__
    #define DosBeep(x,y)
  #else
    #define DosBeep(x,y) do { \
	  sound(x); delay(y); sound(NORM_FREQ); nosound(); } while (0)
  #endif
#endif

struct {
  unsigned char row;
  unsigned char col;
  unsigned char attr;
  char *text;
} bckgrnd[] = {
  { TL_ROW, 28, BG_ATTR, "ARP Backplane Diagnostic"},
  { SG_ROW-2, 12, BG_ATTR, "Signal"},
  { SG_ROW-2, 37, BG_ATTR, "Pin"},
  { SG_ROW-2, 58, BG_ATTR, "State"},
  { SG_ROW-1,  8, BG_ATTR, "ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿"},
  { SG_ROW-1, 33, BG_ATTR, "ÚÄÄÄÄÄÄÄÄÄÄ¿"},
  { SG_ROW-1, 55, BG_ATTR, "ÚÄÄÄÄÄÄÄÄÄÄ¿"},
  { SG_ROW,    8, BG_ATTR, "³   >>>>>>>>   ³"},
  { SG_ROW,   33, BG_ATTR, "³   >>>>   ³"},
  { SG_ROW,   55, BG_ATTR, "³   >>>>   ³"},
  { SG_ROW+1,  8, BG_ATTR, "ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ"},
  { SG_ROW+1, 33, BG_ATTR, "ÀÄÄÄÄÄÄÄÄÄÄÙ"},
  { SG_ROW+1, 55, BG_ATTR, "ÀÄÄÄÄÄÄÄÄÄÄÙ"},
  { SCL_ROW-1, 3, BG_ATTR, "ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿"},
  { SCL_ROW,   3, BG_ATTR, "³ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ÆÍ»"},
  { SCL_ROW+1, 3, BG_ATTR, "ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ º"},
  { SCL_ROW+2, 12, BG_ATTR,         "ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼  Backplane Rev. E"},
  { BPL_ROW-1, 12, BG_ATTR,         "º  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿"},
  { BPL_ROW,  12, BG_ATTR,          "ÈÍÍµ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ³"},
  { BPL_ROW+1, 15, BG_ATTR,            "ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ"},
  { F1_ROW, FT_COL, BG_ATTR, "F1  Audio"},
  { F2_ROW, FT_COL, BG_ATTR, "F2  Pause before each toggle"},
  { F3_ROW, FT_COL, BG_ATTR, "F3  Pause before each line"},
  { F4_ROW, FT_COL, BG_ATTR, "F4  Auto Advance"},
  { SP_ROW, FT_COL-6, BG_ATTR, "+ -       Speed"},
  { F7_ROW, HLP_COL, BG_ATTR, "<pgup>  F7  Previous Bus"},
  { F8_ROW, HLP_COL, BG_ATTR, "<pgdn>  F8  Next Bus"},
  { F9_ROW, HLP_COL, BG_ATTR, "<up>    F9  Previous Signal"},
  { F10_ROW, HLP_COL, BG_ATTR, "<dn>    F10 Next Signal"},
  { 0, 0, 0, NULL }
};

#define SPEED0 5
#define speed (fields[SPDFLD].state)
#define CURR_STATE (fields[STAFLD].state)
#define NOISE_FACTOR 100
unsigned int noise_limit = SPEED0 * NOISE_FACTOR;

struct {
  unsigned char row;
  unsigned char col;
  unsigned char width;
  unsigned int state; /* optional, may be useful to the bitwise fields */
  char *stext[2]; /* If a toggle field, we can do it automatically */
} fields[N_FIELDS] = {
  { SG_ROW, 12, 8, 0, { NULL, NULL }},
  { SG_ROW, 37, 4, 0, { NULL, NULL }},
  { SG_ROW, 59, 4, 0, { "LOW", "HIGH"}},
  { SCL_ROW, 5, 46, 0, { NULL, NULL }},
  { BPL_ROW, 17, 48, 0, { NULL, NULL }},
  { F1_ROW, TG_COL, 3, 0, { "OFF", " ON"}},
  { F2_ROW, TG_COL, 3, 1, { " NO", "YES"}},
  { F3_ROW, TG_COL, 3, 1, { " NO", "YES"}},
  { F4_ROW, TG_COL, 3, 0, { " NO", "YES"}},
  { SP_ROW, TG_COL+1, 1, SPEED0, { NULL, NULL }},
  { SCL_ROW+2, 30, 2, 0, { NULL, NULL }}
};

#define ADDR_BUS 0
#define DATA_BUS 1
#define CTRL_BUS 2
#define EXPN_BUS 3
#define POWR_BUS 4
#define END_BUS  5

/* CTRL_BUS definitions: */
#define CTRL_RD 1
#define CTRL_WR 2
#define CTRL_CE 4
#define CTRL_CS 8

typedef struct {
  unsigned char bus;	/* address, data, cntrl, expen */
  unsigned int bit;
  char *signame;
  char *sc_list;
  char *bp_list;
  char *pname;
  char *cblname;
} siginfo;


siginfo (*signals)[];

siginfo revc_signals[] = {
  { ADDR_BUS, 0x0001, "ADDR0\\", "/U8-18 /U3-9 ³ U3-11 RN2-9 J2-13", "J15-13 RN1-2 U1-2 ³ U1-18 J1-14", "14", "13"},
  { ADDR_BUS, 0x0002, "ADDR1\\", "/U8-19 /U3-8 ³ U3-12 RN2-8 J2-11", "J15-11 RN1-3 U1-4 ³ U1-16 J1-59", "59", "11"},
  { ADDR_BUS, 0x0004, "ADDR2\\", "/U8-20 /U3-7 ³ U3-13 RN2-7 J2-12", "J15-12 RN1-4 U1-6 ³ U1-14 J1-15", "15", "12"},
  { ADDR_BUS, 0x0008, "ADDR3\\", "/U8-21 /U3-6 ³ U3-14 RN2-6 J2-9", "J15-9 RN1-5 U1-8 ³ U1-12 J1-60", "60", "Í9"},
  { ADDR_BUS, 0x0010, "ADDR4\\", "/U8-22 /U3-5 ³ U3-15 RN2-5 J2-10", "J15-10 RN1-6 U1-11 ³ U1-9 J1-16", "16", "10"},
  { ADDR_BUS, 0x0020, "ADDR5\\", "/U8-23 /U3-4 ³ U3-16 RN2-4 J2-7", "J15-7 RN1-7 U1-13 ³ U1-7 J1-61", "61", "Í7"},
  { ADDR_BUS, 0x0040, "ADDR6\\", "/U8-24 /U3-3 ³ U3-17 RN2-3 J2-8", "J15-8 RN1-8 U1-15 ³ U1-5 J1-17", "17", "Í8"},
  { ADDR_BUS, 0x0080, "ADDR7\\", "/U8-25 /U3-2 ³ U3-18 RN2-2 J2-5", "J15-5 RN1-9 U1-17 ³ U1-3 J1-62", "62", "Í5"},
  { ADDR_BUS, 0x0100, "ADDR8\\", "/U18-18 /U24-9 ³ U24-11 RN5-9 J2-30", "J15-30 RN2-2 U2-2 ³ U2-18 J1-18", "18", "30"},
  { ADDR_BUS, 0x0200, "ADDR9\\", "/U18-19 /U24-8 ³ U24-12 RN5-8 J2-29", "J15-29 RN2-3 U2-4 ³ U2-16 J1-63", "63", "29"},
  { ADDR_BUS, 0x0400, "ADDRA\\", "/U18-20 /U24-7 ³ U24-13 RN5-7 J2-28", "J15-28 RN2-4 U2-6 ³ U2-14 J1-19", "19", "28"},
  { ADDR_BUS, 0x0800, "ADDRB\\", "/U18-21 /U24-6 ³ U24-14 RN5-6 J2-23", "J15-23 RN2-5 U2-8 ³ U2-12 J1-64", "64", "23"},
  { ADDR_BUS, 0x1000, "ADDRC\\", "/U18-22 /U24-5 ³ U24-15 RN5-5 J2-24", "J15-24 RN2-6 U2-11 ³ U2-9 J1-20", "20", "24"},
  { ADDR_BUS, 0x2000, "ADDRD\\", "/U18-23 /U24-4 ³ U24-16 RN5-4 J2-25", "J15-25 RN2-7 U2-13 ³ U2-7 J1-65", "65", "25"},
  { ADDR_BUS, 0x4000, "ADDRE\\", "/U18-24 /U24-3 ³ U24-17 RN5-3 J2-26", "J15-26 RN2-8 U2-15 ³ U2-5 J1-21", "21", "26"},
  { ADDR_BUS, 0x8000, "ADDRF\\", "/U18-25 /U24-2 ³ U24-18 RN5-2 J2-27", "J15-27 RN2-9 U2-17 ³ U2-3 J1-66", "66", "27"},
  { DATA_BUS, 0x0001, "DATA0\\", "/U8-4 /U10-9 ³ U10-11 RN3-9 J2-21", "J15-21 RN3-2 U3-9 ³ U3-11 J1-5", "5", "21"},
  { DATA_BUS, 0x0002, "DATA1\\", "/U8-3 /U10-8 ³ U10-12 RN3-8 J2-20", "J15-20 RN3-3 U3-8 ³ U3-12 J1-50", "50", "20"},
  { DATA_BUS, 0x0004, "DATA2\\", "/U8-2 /U10-7 ³ U10-13 RN3-7 J2-19", "J15-19 RN3-4 U3-7 ³ U3-13 J1-6", "6", "19"},
  { DATA_BUS, 0x0008, "DATA3\\", "/U8-1 /U10-6 ³ U10-14 RN3-6 J2-18", "J15-18 RN3-5 U3-6 ³ U3-14 J1-51", "51", "18"},
  { DATA_BUS, 0x0010, "DATA4\\", "/U8-40 /U10-5 ³ U10-15 RN3-5 J2-17", "J15-17 RN3-6 U3-5 ³ U3-15 J1-7", "7", "17"},
  { DATA_BUS, 0x0020, "DATA5\\", "/U8-39 /U10-4 ³ U10-16 RN3-4 J2-16", "J15-16 RN3-7 U3-4 ³ U3-16 J1-52", "52", "16"},
  { DATA_BUS, 0x0040, "DATA6\\", "/U8-38 /U10-3 ³ U10-17 RN3-3 J2-15", "J15-15 RN3-8 U3-3 ³ U3-17 J1-8", "8", "15"},
  { DATA_BUS, 0x0080, "DATA7\\", "/U8-37 /U10-2 ³ U10-18 RN3-2 J2-14", "J15-14 RN3-9 U3-2 ³ U3-18 J1-53", "53", "14"},
  { DATA_BUS, 0x0100, "DATA8\\",  "/U18-4 /U23-9 ³ U23-11 RN6-9 J2-39", "J15-39 RN4-2 U4-9 ³ U4-11 J1-9", "9", "39"},
  { DATA_BUS, 0x0200, "DATA9\\",  "/U18-3 /U23-8 ³ U23-12 RN6-8 J2-40", "J15-40 RN4-3 U4-8 ³ U4-12 J1-54", "54", "40"},
  { DATA_BUS, 0x0400, "DATAA\\",  "/U18-2 /U23-7 ³ U23-13 RN6-7 J2-41", "J15-41 RN4-4 U4-7 ³ U4-13 J1-10", "10", "41"},
  { DATA_BUS, 0x0800, "DATAB\\",  "/U18-1 /U23-6 ³ U23-14 RN6-6 J2-42", "J15-42 RN4-5 U4-6 ³ U4-14 J1-55", "55", "42"},
  { DATA_BUS, 0x1000, "DATAC\\", "/U18-40 /U23-5 ³ U23-15 RN6-5 J2-43", "J15-43 RN4-6 U4-5 ³ U4-15 J1-11", "11", "43"},
  { DATA_BUS, 0x2000, "DATAD\\", "/U18-39 /U23-4 ³ U23-16 RN6-4 J2-44", "J15-44 RN4-7 U4-4 ³ U4-16 J1-56", "56", "44"},
  { DATA_BUS, 0x4000, "DATAE\\", "/U18-38 /U23-3 ³ U23-17 RN6-3 J2-45", "J15-45 RN4-8 U4-3 ³ U4-17 J1-12", "12", "45"},
  { DATA_BUS, 0x8000, "DATAF\\", "/U18-37 /U23-2 ³ U23-18 RN6-2 J2-46", "J15-46 RN4-9 U4-2 ³ U4-18 J1-57", "57", "46"},
  { CTRL_BUS, CTRL_RD, "EXPRD\\", "/U8-14 .. U9-8 U14-7 ³ U14-13 RN4-7 J2-48", "J15-48 RN5-6 U6-15 ³ U6-5 J1-71", "71", "48"},
  { CTRL_BUS, CTRL_WR, "EXPWR\\", "/U8-16 .. U9-11 U14-8 ³ U14-12 RN4-8 J2-49", "J15-49 RN5-7 U6-17 ³ U6-3 J1-25", "25", "49"},
  { CTRL_BUS, CTRL_CE, "CMDENBL\\", "/U5-8 E3 U14-6 ³ U14-14 RN4-6 J2-47", "J15-47 RN5-2 U6-6 ³ U6-14 J1-34", "34", "47"},
  { CTRL_BUS, CTRL_CS, "CMDSTRB\\", "/U8-15 E2 U14-9 ³ U14-11 RN4-9 J2-50", "J15-50 RN5-3 U6-8 ³ U6-12 J1-42", "42", "50"},
  { EXPN_BUS, 0x0000, "EXPEN0\\", "(U28-6 J2-31)",  "(J15-31 RN6-2 JP3) U7-7 U5-2 ³ U5-18 J1-30", "30", "31"},
  { EXPN_BUS, 0x0200, "EXPEN1\\", "(U28-7 J2-32)",  "(J15-32 RN6-3 JP4) U7-9 U5-4 ³ U5-16 J1-75", "75", "32"},
  { EXPN_BUS, 0x0400, "EXPEN2\\", "(U28-8 J2-33)",  "(J15-33 RN6-4 JP5) U7-10 U5-6 ³ U5-14 J1-31", "31", "33"},
  { EXPN_BUS, 0x0600, "EXPEN3\\", "(U28-9 J2-34)",  "(J15-34 RN6-5 JP6) U7-11 U5-8 ³ U5-12 J1-76", "76", "34"},
  { EXPN_BUS, 0x0800, "EXPEN4\\", "(U28-11 J2-35)", "(J15-35 RN6-6 JP7) U7-12 U5-11 ³ U5-9 J1-32", "32", "35"},
  { EXPN_BUS, 0x0A00, "EXPEN5\\", "(U28-12 J2-36)", "(J15-36 RN6-7 JP8) U7-13 U5-13 ³ U5-7 J1-77", "77", "36"},
  { EXPN_BUS, 0x0C00, "EXPEN6\\", "(U28-13 J2-37)", "(J15-37 RN6-8 JP9) U7-14 U5-15 ³ U5-5 J1-33", "33", "37"},
  { EXPN_BUS, 0x0E00, "EXPEN7\\", "(U28-14 J2-38)", "(J15-38 RN6-9 JP10) U7-15 U5-17 ³ U5-3 J1-78", "78", "38"},
  { POWR_BUS, 0x0000, "DIGGND", "J2-3,4,6,53,55,56", "J15-3,4,6,53,55,56 J1-1,24,44,46,67,70", "1", "Í3"},
  { POWR_BUS, 0x0001, "+5V", "J2-1,2", "J15-1,2 (JP1) J1-45,90", "45", "Í1"},
  { POWR_BUS, 0x0000, "ANAGND", "", "J1-4,49", "4", "ÍÍ"},
  { POWR_BUS, 0x0001, "+15V", "", "J1-3,48", "3", "ÍÍ"},
  { POWR_BUS, 0x0001, "-15V", "", "J1-2,47", "2", "ÍÍ"},
  { POWR_BUS, 0x0000, "+28V.RTN", "J2-52", "J15-52 J1-58", "58", "52"},
  { POWR_BUS, 0x0001, "+28V", "J2-54", "J15-54 (JP2) J1-13", "13", "54"},
  { POWR_BUS, 0x0001, "28V.BATT", "", "J1-88", "88", "ÍÍ"},
  { END_BUS, 0, NULL, NULL, NULL, NULL, NULL}
};

siginfo revd_signals[] = {
  { ADDR_BUS, 0x0001, "ADDR0\\", "/U8-18 /U3-9 ³ U3-11 RN2-9 J2-1", "J15-1 RN1-3 U1-17 ³ U1-3 J1-14", "14", "Í1"},
  { ADDR_BUS, 0x0002, "ADDR1\\", "/U8-19 /U3-8 ³ U3-12 RN2-8 J2-2", "J15-2 RN1-2 U1-2 ³ U1-18 J1-59", "59", "Í2"},
  { ADDR_BUS, 0x0004, "ADDR2\\", "/U8-20 /U3-7 ³ U3-13 RN2-7 J2-3", "J15-3 RN1-5 U1-15 ³ U1-5 J1-15", "15", "Í3"},
  { ADDR_BUS, 0x0008, "ADDR3\\", "/U8-21 /U3-6 ³ U3-14 RN2-6 J2-4", "J15-4 RN1-4 U1-4 ³ U1-16 J1-60", "60", "Í4"},
  { ADDR_BUS, 0x0010, "ADDR4\\", "/U8-22 /U3-5 ³ U3-15 RN2-5 J2-5", "J15-5 RN1-7 U1-13 ³ U1-7 J1-16", "16", "Í5"},
  { ADDR_BUS, 0x0020, "ADDR5\\", "/U8-23 /U3-4 ³ U3-16 RN2-4 J2-6", "J15-6 RN1-6 U1-6 ³ U1-14 J1-61", "61", "Í6"},
  { ADDR_BUS, 0x0040, "ADDR6\\", "/U8-24 /U3-3 ³ U3-17 RN2-3 J2-7", "J15-7 RN1-9 U1-11 ³ U1-9 J1-17", "17", "Í7"},
  { ADDR_BUS, 0x0080, "ADDR7\\", "/U8-25 /U3-2 ³ U3-18 RN2-2 J2-8", "J15-8 RN1-8 U1-8 ³ U1-12 J1-62", "62", "Í8"},
  { ADDR_BUS, 0x0100, "ADDR8\\", "/U18-18 /U24-9 ³ U24-11 RN5-9 J2-9", "J15-9 RN2-3 U2-17 ³ U2-3 J1-18", "18", "Í9"},
  { ADDR_BUS, 0x0200, "ADDR9\\", "/U18-19 /U24-8 ³ U24-12 RN5-8 J2-10", "J15-10 RN2-2 U2-2 ³ U2-18 J1-63", "63", "10"},
  { ADDR_BUS, 0x0400, "ADDRA\\", "/U18-20 /U24-7 ³ U24-13 RN5-7 J2-11", "J15-11 RN2-5 U2-15 ³ U2-5 J1-19", "19", "11"},
  { ADDR_BUS, 0x0800, "ADDRB\\", "/U18-21 /U24-6 ³ U24-14 RN5-6 J2-12", "J15-12 RN2-4 U2-4 ³ U2-17 J1-64", "64", "12"},
  { ADDR_BUS, 0x1000, "ADDRC\\", "/U18-22 /U24-5 ³ U24-15 RN5-5 J2-13", "J15-13 RN2-7 U2-13 ³ U2-7 J1-20", "20", "13"},
  { ADDR_BUS, 0x2000, "ADDRD\\", "/U18-23 /U24-4 ³ U24-16 RN5-4 J2-14", "J15-14 RN2-6 U2-6 ³ U2-14 J1-65", "65", "14"},
  { ADDR_BUS, 0x4000, "ADDRE\\", "/U18-24 /U24-3 ³ U24-17 RN5-3 J2-15", "J15-15 RN2-9 U2-11 ³ U2-9 J1-21", "21", "15"},
  { ADDR_BUS, 0x8000, "ADDRF\\", "/U18-25 /U24-2 ³ U24-18 RN5-2 J2-16", "J15-16 RN2-8 U2-8 ³ U2-12 J1-66", "66", "16"},
  { DATA_BUS, 0x0001, "DATA0\\", "/U8-4 /U10-9 ³ U10-11 RN3-9 J2-21", "J15-21 RN3-9 U3-9 ³ U3-11 J1-5", "5", "21"},
  { DATA_BUS, 0x0002, "DATA1\\", "/U8-3 /U10-8 ³ U10-12 RN3-8 J2-22", "J15-22 RN3-8 U3-8 ³ U3-12 J1-50", "50", "22"},
  { DATA_BUS, 0x0004, "DATA2\\", "/U8-2 /U10-7 ³ U10-13 RN3-7 J2-23", "J15-23 RN3-7 U3-7 ³ U3-13 J1-6", "6", "23"},
  { DATA_BUS, 0x0008, "DATA3\\", "/U8-1 /U10-6 ³ U10-14 RN3-6 J2-24", "J15-24 RN3-6 U3-6 ³ U3-14 J1-51", "51", "24"},
  { DATA_BUS, 0x0010, "DATA4\\", "/U8-40 /U10-5 ³ U10-15 RN3-5 J2-25", "J15-25 RN3-5 U3-5 ³ U3-15 J1-7", "7", "25"},
  { DATA_BUS, 0x0020, "DATA5\\", "/U8-39 /U10-4 ³ U10-16 RN3-4 J2-26", "J15-26 RN3-4 U3-4 ³ U3-16 J1-52", "52", "26"},
  { DATA_BUS, 0x0040, "DATA6\\", "/U8-38 /U10-3 ³ U10-17 RN3-3 J2-27", "J15-27 RN3-3 U3-3 ³ U3-17 J1-8", "8", "27"},
  { DATA_BUS, 0x0080, "DATA7\\", "/U8-37 /U10-2 ³ U10-18 RN3-2 J2-28", "J15-28 RN3-2 U3-2 ³ U3-18 J1-53", "53", "28"},
  { DATA_BUS, 0x0100, "DATA8\\",  "/U18-4 /U23-9 ³ U23-11 RN6-9 J2-29", "J15-29 RN4-9 U4-9 ³ U4-11 J1-9", "9", "29"},
  { DATA_BUS, 0x0200, "DATA9\\",  "/U18-3 /U23-8 ³ U23-12 RN6-8 J2-30", "J15-30 RN4-8 U4-8 ³ U4-12 J1-54", "54", "30"},
  { DATA_BUS, 0x0400, "DATAA\\",  "/U18-2 /U23-7 ³ U23-13 RN6-7 J2-31", "J15-31 RN4-7 U4-7 ³ U4-13 J1-10", "10", "31"},
  { DATA_BUS, 0x0800, "DATAB\\",  "/U18-1 /U23-6 ³ U23-14 RN6-6 J2-32", "J15-32 RN4-6 U4-6 ³ U4-14 J1-55", "55", "32"},
  { DATA_BUS, 0x1000, "DATAC\\", "/U18-40 /U23-5 ³ U23-15 RN6-5 J2-33", "J15-33 RN4-5 U4-5 ³ U4-15 J1-11", "11", "33"},
  { DATA_BUS, 0x2000, "DATAD\\", "/U18-39 /U23-4 ³ U23-16 RN6-4 J2-34", "J15-34 RN4-4 U4-4 ³ U4-16 J1-56", "56", "34"},
  { DATA_BUS, 0x4000, "DATAE\\", "/U18-38 /U23-3 ³ U23-17 RN6-3 J2-35", "J15-35 RN4-3 U4-3 ³ U4-17 J1-12", "12", "35"},
  { DATA_BUS, 0x8000, "DATAF\\", "/U18-37 /U23-2 ³ U23-18 RN6-2 J2-36", "J15-36 RN4-2 U4-2 ³ U4-18 J1-57", "57", "36"},
  { CTRL_BUS, CTRL_RD, "EXPRD\\", "/U8-14 .. U9-8 U14-6 ³ U14-14 RN4-7 J2-46", "J15-46 RN5-9 U6-15 ³ U6-5 J1-71", "71", "46"},
  { CTRL_BUS, CTRL_WR, "EXPWR\\", "/U8-16 .. U9-11 U14-4 ³ U14-16 RN4-8 J2-48", "J15-48 RN5-7 U6-17 ³ U6-3 J1-25", "25", "48"},
  { CTRL_BUS, CTRL_CE, "CMDENBL\\", "/U5-8 E3 U14-8 ³ U14-12 RN4-6 J2-52", "J15-52 RN5-4 U6-6 ³ U6-14 J1-34", "34", "52"},
  { CTRL_BUS, CTRL_CS, "CMDSTRB\\", "/U8-15 E2 U14-2 ³ U14-18 RN4-9 J2-50", "J15-50 RN5-5 U6-8 ³ U6-12 J1-42", "42", "50"},
  { EXPN_BUS, 0x0000, "EXPEN0\\", "",  "U7-7 U5-17 ³ U5-3 J1-30", "30", "ÍÍ"},
  { EXPN_BUS, 0x0200, "EXPEN1\\", "",  "U7-9 U5-2 ³ U5-18 J1-75", "75", "ÍÍ"},
  { EXPN_BUS, 0x0400, "EXPEN2\\", "",  "U7-10 U5-15 ³ U5-5 J1-31", "31", "ÍÍ"},
  { EXPN_BUS, 0x0600, "EXPEN3\\", "",  "U7-11 U5-4 ³ U5-16 J1-76", "76", "ÍÍ"},
  { EXPN_BUS, 0x0800, "EXPEN4\\", "", "U7-12 U5-13 ³ U5-7 J1-32", "32", "ÍÍ"},
  { EXPN_BUS, 0x0A00, "EXPEN5\\", "", "U7-13 U5-6 ³ U5-14 J1-77", "77", "ÍÍ"},
  { EXPN_BUS, 0x0C00, "EXPEN6\\", "", "U7-14 U5-11 ³ U5-9 J1-33", "33", "ÍÍ"},
  { EXPN_BUS, 0x0E00, "EXPEN7\\", "", "U7-15 U5-8 ³ U5-12 J1-78", "78", "ÍÍ"},
  { POWR_BUS, 0x0000, "DIGGND", "J2-45,47,49,51,53,55", "J15-45,47,49,51,53,55 J1-1,24,44,46,67,70", "1", "45"},
  { POWR_BUS, 0x0001, "+5V", "(J2-17,18)", "(J15-17,18) (JP1) J1-45,90", "45", "17"},
  { POWR_BUS, 0x0000, "ANAGND", "", "J1-4,49", "4", "ÍÍ"},
  { POWR_BUS, 0x0001, "+15V", "", "J1-3,48", "3", "ÍÍ"},
  { POWR_BUS, 0x0001, "-15V", "", "J1-2,47", "2", "ÍÍ"},
  { POWR_BUS, 0x0000, "+28V.RTN", "J2-19", "J15-19 J1-58", "58", "19"},
  { POWR_BUS, 0x0001, "+28V", "J2-20", "J15-20 (JP2) J1-13", "13", "20"},
  { POWR_BUS, 0x0001, "28V.BATT", "", "J1-88", "88", "ÍÍ"},
  { END_BUS, 0, NULL, NULL, NULL, NULL, NULL}
};

siginfo sc104_signals[] = {
  { ADDR_BUS, 0x0001, "ADDR0\\", "/U8-18 /U3-9 ³ U3-11 RN2-9 J2-1", "J15-1 RN1-3 U1-17 ³ U1-3 J1-14", "14", "Í1"},
  { ADDR_BUS, 0x0002, "ADDR1\\", "/U8-19 /U3-8 ³ U3-12 RN2-8 J2-2", "J15-2 RN1-2 U1-2 ³ U1-18 J1-59", "59", "Í2"},
  { ADDR_BUS, 0x0004, "ADDR2\\", "/U8-20 /U3-7 ³ U3-13 RN2-7 J2-3", "J15-3 RN1-5 U1-15 ³ U1-5 J1-15", "15", "Í3"},
  { ADDR_BUS, 0x0008, "ADDR3\\", "/U8-21 /U3-6 ³ U3-14 RN2-6 J2-4", "J15-4 RN1-4 U1-4 ³ U1-16 J1-60", "60", "Í4"},
  { ADDR_BUS, 0x0010, "ADDR4\\", "/U8-22 /U3-5 ³ U3-15 RN2-5 J2-5", "J15-5 RN1-7 U1-13 ³ U1-7 J1-16", "16", "Í5"},
  { ADDR_BUS, 0x0020, "ADDR5\\", "/U8-23 /U3-4 ³ U3-16 RN2-4 J2-6", "J15-6 RN1-6 U1-6 ³ U1-14 J1-61", "61", "Í6"},
  { ADDR_BUS, 0x0040, "ADDR6\\", "/U8-24 /U3-3 ³ U3-17 RN2-3 J2-7", "J15-7 RN1-9 U1-11 ³ U1-9 J1-17", "17", "Í7"},
  { ADDR_BUS, 0x0080, "ADDR7\\", "/U8-25 /U3-2 ³ U3-18 RN2-2 J2-8", "J15-8 RN1-8 U1-8 ³ U1-12 J1-62", "62", "Í8"},
  { ADDR_BUS, 0x0100, "ADDR8\\", "/U18-18 /U24-9 ³ U24-11 RN5-9 J2-9", "J15-9 RN2-3 U2-17 ³ U2-3 J1-18", "18", "Í9"},
  { ADDR_BUS, 0x0200, "ADDR9\\", "/U18-19 /U24-8 ³ U24-12 RN5-8 J2-10", "J15-10 RN2-2 U2-2 ³ U2-18 J1-63", "63", "10"},
  { ADDR_BUS, 0x0400, "ADDRA\\", "/U18-20 /U24-7 ³ U24-13 RN5-7 J2-11", "J15-11 RN2-5 U2-15 ³ U2-5 J1-19", "19", "11"},
  { ADDR_BUS, 0x0800, "ADDRB\\", "/U18-21 /U24-6 ³ U24-14 RN5-6 J2-12", "J15-12 RN2-4 U2-4 ³ U2-17 J1-64", "64", "12"},
  { ADDR_BUS, 0x1000, "ADDRC\\", "/U18-22 /U24-5 ³ U24-15 RN5-5 J2-13", "J15-13 RN2-7 U2-13 ³ U2-7 J1-20", "20", "13"},
  { ADDR_BUS, 0x2000, "ADDRD\\", "/U18-23 /U24-4 ³ U24-16 RN5-4 J2-14", "J15-14 RN2-6 U2-6 ³ U2-14 J1-65", "65", "14"},
  { ADDR_BUS, 0x4000, "ADDRE\\", "/U18-24 /U24-3 ³ U24-17 RN5-3 J2-15", "J15-15 RN2-9 U2-11 ³ U2-9 J1-21", "21", "15"},
  { ADDR_BUS, 0x8000, "ADDRF\\", "/U18-25 /U24-2 ³ U24-18 RN5-2 J2-16", "J15-16 RN2-8 U2-8 ³ U2-12 J1-66", "66", "16"},
  { DATA_BUS, 0x0001, "DATA0\\", "/U8-4 /U10-9 ³ U10-11 RN3-9 J2-21", "J15-21 RN3-9 U3-9 ³ U3-11 J1-5", "5", "21"},
  { DATA_BUS, 0x0002, "DATA1\\", "/U8-3 /U10-8 ³ U10-12 RN3-8 J2-22", "J15-22 RN3-8 U3-8 ³ U3-12 J1-50", "50", "22"},
  { DATA_BUS, 0x0004, "DATA2\\", "/U8-2 /U10-7 ³ U10-13 RN3-7 J2-23", "J15-23 RN3-7 U3-7 ³ U3-13 J1-6", "6", "23"},
  { DATA_BUS, 0x0008, "DATA3\\", "/U8-1 /U10-6 ³ U10-14 RN3-6 J2-24", "J15-24 RN3-6 U3-6 ³ U3-14 J1-51", "51", "24"},
  { DATA_BUS, 0x0010, "DATA4\\", "/U8-40 /U10-5 ³ U10-15 RN3-5 J2-25", "J15-25 RN3-5 U3-5 ³ U3-15 J1-7", "7", "25"},
  { DATA_BUS, 0x0020, "DATA5\\", "/U8-39 /U10-4 ³ U10-16 RN3-4 J2-26", "J15-26 RN3-4 U3-4 ³ U3-16 J1-52", "52", "26"},
  { DATA_BUS, 0x0040, "DATA6\\", "/U8-38 /U10-3 ³ U10-17 RN3-3 J2-27", "J15-27 RN3-3 U3-3 ³ U3-17 J1-8", "8", "27"},
  { DATA_BUS, 0x0080, "DATA7\\", "/U8-37 /U10-2 ³ U10-18 RN3-2 J2-28", "J15-28 RN3-2 U3-2 ³ U3-18 J1-53", "53", "28"},
  { DATA_BUS, 0x0100, "DATA8\\",  "/U18-4 /U23-9 ³ U23-11 RN6-9 J2-29", "J15-29 RN4-9 U4-9 ³ U4-11 J1-9", "9", "29"},
  { DATA_BUS, 0x0200, "DATA9\\",  "/U18-3 /U23-8 ³ U23-12 RN6-8 J2-30", "J15-30 RN4-8 U4-8 ³ U4-12 J1-54", "54", "30"},
  { DATA_BUS, 0x0400, "DATAA\\",  "/U18-2 /U23-7 ³ U23-13 RN6-7 J2-31", "J15-31 RN4-7 U4-7 ³ U4-13 J1-10", "10", "31"},
  { DATA_BUS, 0x0800, "DATAB\\",  "/U18-1 /U23-6 ³ U23-14 RN6-6 J2-32", "J15-32 RN4-6 U4-6 ³ U4-14 J1-55", "55", "32"},
  { DATA_BUS, 0x1000, "DATAC\\", "/U18-40 /U23-5 ³ U23-15 RN6-5 J2-33", "J15-33 RN4-5 U4-5 ³ U4-15 J1-11", "11", "33"},
  { DATA_BUS, 0x2000, "DATAD\\", "/U18-39 /U23-4 ³ U23-16 RN6-4 J2-34", "J15-34 RN4-4 U4-4 ³ U4-16 J1-56", "56", "34"},
  { DATA_BUS, 0x4000, "DATAE\\", "/U18-38 /U23-3 ³ U23-17 RN6-3 J2-35", "J15-35 RN4-3 U4-3 ³ U4-17 J1-12", "12", "35"},
  { DATA_BUS, 0x8000, "DATAF\\", "/U18-37 /U23-2 ³ U23-18 RN6-2 J2-36", "J15-36 RN4-2 U4-2 ³ U4-18 J1-57", "57", "36"},
  { CTRL_BUS, CTRL_RD, "EXPRD\\", "/U8-14 .. U9-8 U14-6 ³ U14-14 RN4-7 J2-46", "J15-46 RN5-9 U6-15 ³ U6-5 J1-71", "71", "46"},
  { CTRL_BUS, CTRL_WR, "EXPWR\\", "/U8-16 .. U9-11 U14-4 ³ U14-16 RN4-8 J2-48", "J15-48 RN5-7 U6-17 ³ U6-3 J1-25", "25", "48"},
  { CTRL_BUS, CTRL_CE, "CMDENBL\\", "/U5-8 E3 U14-8 ³ U14-12 RN4-6 J2-52", "J15-52 RN5-4 U6-6 ³ U6-14 J1-34", "34", "52"},
  { CTRL_BUS, CTRL_CS, "CMDSTRB\\", "/U8-15 E2 U14-2 ³ U14-18 RN4-9 J2-50", "J15-50 RN5-5 U6-8 ³ U6-12 J1-42", "42", "50"},
  { EXPN_BUS, 0x0000, "EXPEN0\\", "",  "U7-7 U5-17 ³ U5-3 J1-30", "30", "ÍÍ"},
  { EXPN_BUS, 0x0200, "EXPEN1\\", "",  "U7-9 U5-2 ³ U5-18 J1-75", "75", "ÍÍ"},
  { EXPN_BUS, 0x0400, "EXPEN2\\", "",  "U7-10 U5-15 ³ U5-5 J1-31", "31", "ÍÍ"},
  { EXPN_BUS, 0x0600, "EXPEN3\\", "",  "U7-11 U5-4 ³ U5-16 J1-76", "76", "ÍÍ"},
  { EXPN_BUS, 0x0800, "EXPEN4\\", "", "U7-12 U5-13 ³ U5-7 J1-32", "32", "ÍÍ"},
  { EXPN_BUS, 0x0A00, "EXPEN5\\", "", "U7-13 U5-6 ³ U5-14 J1-77", "77", "ÍÍ"},
  { EXPN_BUS, 0x0C00, "EXPEN6\\", "", "U7-14 U5-11 ³ U5-9 J1-33", "33", "ÍÍ"},
  { EXPN_BUS, 0x0E00, "EXPEN7\\", "", "U7-15 U5-8 ³ U5-12 J1-78", "78", "ÍÍ"},
  { POWR_BUS, 0x0000, "DIGGND", "J2-45,47,49,51,53,55", "J15-45,47,49,51,53,55 J1-1,24,44,46,67,70", "1", "45"},
  { POWR_BUS, 0x0001, "+5V", "(J2-17,18)", "(J15-17,18) (JP1) J1-45,90", "45", "17"},
  { POWR_BUS, 0x0000, "ANAGND", "", "J1-4,49", "4", "ÍÍ"},
  { POWR_BUS, 0x0001, "+15V", "", "J1-3,48", "3", "ÍÍ"},
  { POWR_BUS, 0x0001, "-15V", "", "J1-2,47", "2", "ÍÍ"},
  { POWR_BUS, 0x0000, "+28V.RTN", "J2-19", "J15-19 J1-58", "58", "19"},
  { POWR_BUS, 0x0001, "+28V", "J2-20", "J15-20 (JP2) J1-13", "13", "20"},
  { POWR_BUS, 0x0001, "28V.BATT", "", "J1-88", "88", "ÍÍ"},
  { END_BUS, 0, NULL, NULL, NULL, NULL, NULL}
};

#define beeping() fields[AUDFLD].state
#define pause_on_toggle() fields[TPSFLD].state
#define pause_on_line() fields[LPSFLD].state
#define auto_advance() fields[AAFLD].state

void write_field(int fldno, char *text) {
  int i;

  assert(fldno < N_FIELDS);
  assert(strlen(text) <= fields[fldno].width);
  (void)mvaddstr(fields[fldno].row, fields[fldno].col, text);
  for (i = fields[fldno].width - strlen(text); i > 0; i--)
    addch(' ');
}

void update_toggle(int fldno) {
  assert(fields[fldno].state == 0 || fields[fldno].state == 1);
  write_field(fldno, fields[fldno].stext[fields[fldno].state]);
}

void toggle_toggle(int fldno) {
  fields[fldno].state ^= 1;
  update_toggle(fldno);
  refresh();
}

void update_number(int fldno) {
  char buf[30];

  sprintf(buf, "%d", fields[fldno].state);
  write_field(fldno, buf);
}

/* svc_kbd(void) sets bits in the global flags word kbd_flags and
   returns the kbd_flags word.
*/
unsigned int kbd_flags = 0;
#define KBF_NXT_SIG 1
#define KBF_PRV_SIG 2
#define KBF_NXT_BUS 4
#define KBF_PRV_BUS 8
#define KBF_PROCEED 0x10
#define KBF_EXIT    0x20
#define KBF_PROCKEYS 0x3F
#define KBF_SIGKEYS 0xF
#define KBF_ESIGKEYS 0x2F

unsigned int svc_kbd(void) {
  int c;
  
  c = getch();
  switch (c) {
    case '\033':
      kbd_flags |= KBF_EXIT;
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      speed = c - '0';
      noise_limit = speed * NOISE_FACTOR;
      update_number(SPDFLD);
      refresh();
      break;
    case '+':
      if (speed < 9) {
        speed++;
        noise_limit = speed * NOISE_FACTOR;
        update_number(SPDFLD);
        refresh();
      }
      break;
    case '-':
      if (speed > 0) {
        speed--;
        noise_limit = speed * NOISE_FACTOR;
        update_number(SPDFLD);
        refresh();
      }
      break;
    case KEY_F1:
      toggle_toggle(AUDFLD);
      break;
    case KEY_F2:
      toggle_toggle(TPSFLD);
      break;
    case KEY_F3:
      toggle_toggle(LPSFLD);
      break;
    case KEY_F4:
      toggle_toggle(AAFLD);
      break;
    case KEY_F7:
    case KEY_PGUP:
      kbd_flags |= KBF_PRV_BUS;
      break;
    case KEY_F8:
    case KEY_PGDN:
      kbd_flags |= KBF_NXT_BUS;
      break;
    case KEY_F9:
    case KEY_UP:
      kbd_flags |= KBF_PRV_SIG;
      break;
    case KEY_F10:
    case KEY_DOWN:
      kbd_flags |= KBF_NXT_SIG;
      break;
    case ERR:
      break;
    default:
      kbd_flags |= KBF_PROCEED;
      break;
  }
  return(kbd_flags);
}

/* State in these four functions is the logical state for the function
   named, not the level read on the backplane. Hence sb_cmdenbl(1)
   asks to assert cmdenbl, or make CMDENBL\ low on the backplane.
*/
void sb_read(int state) {
  if (state != 0) state = 1;
  if ( ver == VER_SC104 ) {
	if ( state ) SB104_Ctrl |= 1;
	else SB104_Ctrl &= ~1;
	outbyte( 0x30C, SB104_Ctrl );
  } else outbyte(SC_SB_LOWCTRL, state);
}

void sb_write(int state) {
  if (state != 0) state = 1;
  if ( ver == VER_SC104 ) {
	if ( state ) SB104_Ctrl |= 4;
	else SB104_Ctrl &= ~4;
	outbyte( 0x30C, SB104_Ctrl );
  } else outbyte(SC_SB_LOWCTRL, state | 4);
}

void sb_cmdenbl(int state) {
  if (state != 0) {
    outbyte(SC_CMDENBL, 1);
    tick_sic();
  } else disable_sic();
}

void sb_cmdstrb(int state) {
  if (state != 0) state = 1;
  if ( ver == VER_SC104 ) {
	if ( state ) SB104_Ctrl &= ~2;
	else SB104_Ctrl |= 2;
	outbyte( 0x30C, SB104_Ctrl | 8 );
  } else outbyte(SC_SB_LOWCTRL, state | 2);
}

/* sb_ctrl drives all of the control signals to set positions */
void sb_ctrl(unsigned int mask) {
  sb_read(mask & CTRL_RD);
  sb_write(mask & CTRL_WR);
  sb_cmdenbl(mask & CTRL_CE);
  sb_cmdstrb(mask & CTRL_CS);
}

void drive_sig(int signo) {
  siginfo *sig;
  unsigned int mask;
  
  sig = &(*signals)[signo];
  if (sig->bus == POWR_BUS) CURR_STATE = sig->bit;
  mask = (CURR_STATE ? 0 : ~0);
  if (sig->bus != ADDR_BUS && sig->bus != EXPN_BUS) sb_addr(mask);
  if (sig->bus != DATA_BUS) sb_data(mask);
  if (sig->bus != CTRL_BUS) sb_ctrl(0);
  switch (sig->bus) {
    case ADDR_BUS:
      sb_addr(mask ^ sig->bit);
      break;
    case DATA_BUS:
      sb_data(mask ^ sig->bit);
      break;
    case CTRL_BUS:
      sb_ctrl(sig->bit & mask);
      break;
    case EXPN_BUS:
      sb_addr((~mask) ^ sig->bit);
      break;
    case POWR_BUS:
      break;
  }
}

void make_noise(int signo) {
  siginfo *sig;
  unsigned int addr_mask, addr_val;
  unsigned int data_mask, data_val;
  unsigned int ctrl_mask, ctrl_val;
  unsigned int count;
  
  sig = &(*signals)[signo];
  addr_mask = data_mask = ctrl_mask = (unsigned int)(~0);
  addr_val = data_val = ctrl_val = 0;
  switch (sig->bus) {
    case ADDR_BUS:
      addr_mask = ~sig->bit;
      addr_val = CURR_STATE ? 0 : sig->bit;
      break;
    case DATA_BUS:
      data_mask = ~sig->bit;
      data_val = CURR_STATE ? 0 : sig->bit;
      ctrl_mask &= ~1; /* don't toggle the read line */
      break;
    case CTRL_BUS:
      ctrl_mask = ~sig->bit;
      ctrl_val = CURR_STATE ? 0 : sig->bit;
      if (CURR_STATE == 0) {
        if (sig->bit == CTRL_RD) ctrl_mask &= ~CTRL_WR;
        else if (sig->bit == CTRL_WR) ctrl_mask &= ~CTRL_RD;
        else if (sig->bit == CTRL_CE) tick_sic();
      }
      break;
    case EXPN_BUS:
      addr_mask = 0x1FF;
      addr_val = (CURR_STATE ? 0xE00 : 0 ) ^ sig->bit;
      break;
  }
  for (count = 0; count < 4; count++) {
    sb_addr(((count & 1) ? addr_mask : 0) | addr_val);
    sb_data(((count & 2) ? data_mask : 0) | data_val);
    if (ctrl_mask & CTRL_RD) {
      sb_read(1);
      sb_read(0);
    }
    if (ctrl_mask & CTRL_WR) {
      sb_write(1);
      sb_write(0);
    }
    if (ctrl_mask & CTRL_CE) {
      sb_cmdenbl(1);
      sb_cmdenbl(0);
    }
    if (ctrl_mask & CTRL_CS) {
      sb_cmdstrb(1);
      sb_cmdstrb(0);
    }
  }
}

void init_subbus( void ) {  

  #ifdef __QNXNTO__
    if ( ThreadCtl(_NTO_TCTL_IO, 0) == -1 ) {
      fprintf(stderr,
	"Unable to gain I/O privileges. Perhaps not root?\n" );
      exit(1);
    }
  #endif
  /* Initialize the subbus */
  outbyte(0x310, 0);
  if ( ver == VER_PCICC ) {
	outbyte(0x30B, 0xC0);
	outbyte(0x30F, 0xC1);
  } else {
	outword(0x30E, 0xC1C0);
  }
  
  /* Determine if this is Syscon104 */
  /* If Syscon104, redefine ver */
  if ( ver == VER_REVD ) {
	unsigned short testpat, readback;
	testpat = 0x5555;
	sb_addr( testpat );
	readback = inword( 0x30A );
	if ( readback == 0x4B01 ) {
	  ver = VER_SC104;
	  outbyte( 0x30C, 0x10 ); /* disable buffers! */
	  /* disable timeouts and auto-write for diagnostics */
	  outword( 0x312, 0x1940 ); /* diagnostic configuration */
	  readback = inword( 0x30C );
	  if ( (readback & 0x20 ) == 0 ) {
		if ( enable_sc104_bufs ) {
		  int c;
		  
		  printf( "Warning:\a Enabling buffers can damage hardware!\n"
			"Verify that ICC is powered before proceeding!\n"
			"Continue? [n/y] " );
		  fflush( stdout );
		  for (;;) {
			c = getchar();
			if ( c == 'y' || c == 'Y' ) break;
			if ( c == 'n' || c == 'N' || c == '\n' || c == '\r' )
			  exit(1);
		  }
		  outbyte( 0x30C, 0x30 );
		} else {
		  fprintf( stderr, "Syscon 104 buffers not enabled\n" );
		  exit(1);
		}
	  }
	}
  }
}

void operate(void) {
  int signo;
  siginfo *sig;
  unsigned int noise_count;
  unsigned int this_bus, toggle_count;

  /* Main loop for each signal */
  for (signo = 0;;) {
    sig = &(*signals)[signo];

    /* display this signal's pertinent info */
    write_field(SIGFLD, sig->signame);
    write_field(PINFLD, sig->pname);
    write_field(SCLIST, sig->sc_list);
    write_field(BPLIST, sig->bp_list);
    write_field(CBLFLD, sig->cblname);

    /* drive line high and update state */
    CURR_STATE = 1;
    drive_sig(signo);
    update_toggle(STAFLD);
    refresh();

    /* Wait if we're supposed to */
    while (pause_on_line() && !pause_on_toggle() && svc_kbd() == 0);
    kbd_flags &= ~KBF_PROCEED;
    
    if ((kbd_flags & KBF_ESIGKEYS) == 0) for (toggle_count = 4;;) {
      /* Beep for current state */
      if (beeping())
        DosBeep(CURR_STATE ? HIGH_FREQ : LOW_FREQ, beep_dur);

      for (noise_count = noise_limit;;) {
        make_noise(signo);
        svc_kbd();
	if (kbd_flags & KBF_PROCKEYS) break;
        if (!pause_on_toggle() && noise_count-- == 0) break;
      }
      kbd_flags &= ~KBF_PROCEED;
      if (kbd_flags & KBF_ESIGKEYS) break;
      if (auto_advance() && --toggle_count <= 0) break;

      /* Drive line to next state */
      CURR_STATE ^= 1;
      drive_sig(signo);
      update_toggle(STAFLD);
      refresh();
    }

    /* Advance to the next signal, depending on kbd_flags */
    if (kbd_flags & KBF_EXIT) break;
    if (kbd_flags & KBF_NXT_BUS) {
      this_bus = (*signals)[signo].bus;
      do {
        signo++;
		if ( (*signals)[signo].bus == END_BUS ) signo = 0;
      } while ((*signals)[signo].bus == this_bus);
    } else {
      if (kbd_flags & (KBF_PRV_SIG|KBF_PRV_BUS)) {
		if ( signo > 0 ) signo--;
		else {
		  for ( signo++; (*signals)[ signo ].bus != END_BUS; signo++ );
		  signo--;
		}
      } else {
        signo++;
		if ( (*signals)[signo].bus == END_BUS ) signo = 0;
      }
      if (kbd_flags & KBF_PRV_BUS) {
        this_bus = (*signals)[signo].bus;
        while (signo > 0 && (*signals)[signo-1].bus == this_bus) signo--;
      }
    }
    kbd_flags &= ~KBF_SIGKEYS;
  }
  disable_sic();
}

int main( int argc, char **argv ) {
  int i;

  oui_init_options( argc, argv );
  if ( ver == VER_PCICC ) {
	SC_SB_LOWCTRL = 0x30B;
	SC_CMDENBL = 0x311;
	signals = revc_signals;
  } else {
	SC_SB_LOWCTRL = 0x30E;
	SC_CMDENBL = 0x318;
	if ( ver == VER_REVC ) signals = revc_signals;
	else signals = revd_signals;
  }
  
  init_subbus(); /* detects SC104 */
  if ( ver == VER_SC104 ) {
	signals = sc104_signals;
  }

  /* initialize curses and select our options */
  if (initscr() == ERR) {
    printf("Cannot init screen");
    exit(1);
  }
  scrollok(stdscr, 0);
  nodelay(stdscr, 1);
  raw();
  noecho();
  keypad(stdscr, 1);

  /* Draw the background */
  for (i = 0; bckgrnd[i].text != NULL; i++) {
    attrset(bckgrnd[i].attr);
    (void)mvaddstr(bckgrnd[i].row, bckgrnd[i].col, bckgrnd[i].text);
  }
  { char *title;

	attrset( BG_ATTR );
	switch ( ver ) {
	  case VER_PCICC: title = " PC/ICC Interface Board "; break;
	  case VER_REVC:  title = "System Controller Rev. C"; break;
	  case VER_REVD:  title = "System Controller Rev. D"; break;
	  case VER_SC104: title = "  System Controller/104 "; break;
	}
	(void) mvaddstr( SCL_ROW-2, 4, title );
  }
  
  /* Initialize the fields */
  update_toggle(STAFLD);
  update_toggle(AUDFLD);
  update_toggle(TPSFLD);
  update_toggle(LPSFLD);
  update_toggle(AAFLD);
  update_number(SPDFLD);

  /* Operate */
  operate();

  if ( ver == VER_SC104 )
	outbyte(0x310, 0); /* reset to default configuration */

  /* Terminate */
  clear();
  refresh();
  endwin();
  return(0);
}
