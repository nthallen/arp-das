/** @file
 subbus.h defines the interface to the subbus resident library
 Before calling the subbus routines, you must first call
 load_subbus().  This returns the subfunction of the resident
 subbus library or 0 if none is installed.
 */
#ifndef SUBBUS_H_INCLUDED
#define SUBBUS_H_INCLUDED
#include <sys/siginfo.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Subbus subfunction codes: These define the hardware that talks
   to the subbus. It does not talk about the interface between
   the library and the system controller itself.
 */
#define SB_PCICC 1
#define SB_PCICCSIC 2
#define SB_SYSCON 3
#define SB_SYSCON104 4
#define SB_SYSCONDACS 5

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

extern unsigned short subbus_version;
extern unsigned short subbus_features;
extern unsigned short subbus_subfunction;
extern unsigned short read_subbus(unsigned short addr);
extern int write_ack(unsigned short addr, unsigned short data);
extern int read_ack(unsigned short addr, unsigned short *data);
#define write_subbus(x,y) write_ack(x,y)
extern int set_cmdenbl(int value);
extern int set_cmdstrobe(int value);
extern unsigned short read_switches(void);
extern int set_failure(unsigned short value);
extern unsigned short read_failure(void);
extern int  tick_sic(void);
extern int disarm_sic(void);
extern char *get_subbus_name(void);
#define subbus_name get_subbus_name()
extern int cache_write(unsigned short addr, unsigned short data);
extern unsigned short cache_read(unsigned short addr);

extern unsigned short sbrb(unsigned short addr);
extern unsigned short sbrba(unsigned short addr);
#define sbrw(x) read_subbus(x)
extern unsigned int sbrwa(unsigned short addr);
#define sbwr(x,y) write_ack(x,y)
#define sbwra(x,y) write_ack(x,y)

extern int subbus_int_attach( char *cardID, unsigned short address,
      unsigned short region, struct sigevent *event );
extern int subbus_int_detach( char *cardID );
extern int subbus_quit(void);

#ifdef __cplusplus
};
#endif

#endif
