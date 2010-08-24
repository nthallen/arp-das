/* subbus.h defines the interface to the subbus resident library
 * Before calling the subbus routines, you must first call
 * load_subbus().  This returns the subfunction of the resident
 * subbus library or 0 if none is installed.  If you call a
 * subbus function without initializing, or if the initialization
 * fails, you are guaranteed to crash.
 */
#ifndef SUBBUS_H_INCLUDED
#define SUBBUS_H_INCLUDED
#include <sys/siginfo.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Subbus version codes */
#define SB_PCICC 1
#define SB_PCICCSIC 2
#define SB_SYSCON 3
#define SB_SYSCON104 4
#define SB_SYSCOND 5

/* subbus_features: */
#define SBF_SIC 1		/* SIC Functions */
#define SBF_LG_RAM 2	/* Large NVRAM */
#define SBF_HW_CNTS 4	/* Hardware rst & pwr Counters */
#define SBF_WD 8		/* Watchdog functions */
#define SBF_SET_FAIL 0x10 /* Set failure lamp */
#define SBF_READ_FAIL 0x20 /* Read failure lamps */
#define SBF_READ_SW 0x40 /* Read Switches */
#define SBF_NVRAM 0x80   /* Any NVRAM at all! */
#define SBF_CMDSTROBE 0x100 /* CmdStrobe Function */

extern int load_subbus(void);

extern unsigned int subbus_version;
extern unsigned int subbus_features;
extern unsigned int subbus_subfunction;
extern unsigned short read_subbus(unsigned short addr);
extern int write_ack(unsigned short addr, unsigned short data);
extern int read_ack(unsigned short addr, unsigned short *data);
#define write_subbus(x,y) write_ack(x,y)
extern void set_cmdenbl(int value);
extern unsigned int read_switches(void);
extern void set_failure(int value);
extern unsigned char read_failure(void);
extern short int set_cmdstrobe(short int value);
extern int  tick_sic(void);
extern void disarm_sic(void);
extern char *get_subbus_name(void);
#define subbus_name get_subbus_name()

#define sbw(x) read_subbus(x)
#define sbwr(x,y) write_ack(x,y)
#define sbwra(x,y) write_ack(x,y)
unsigned int sbb(unsigned short addr);
unsigned int sbba(unsigned short addr);
unsigned int sbwa(unsigned short addr);

int subbus_int_attach( char *cardID, unsigned short address,
      unsigned short region, struct sigevent *event );
int subbus_int_detach( char *cardID );

#ifdef __cplusplus
};
#endif

#endif
