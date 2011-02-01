#ifndef SC104_H_INCLUDED
#define SC104_H_INCLUDED
#include <sys/dispatch.h>
#include "subbusd.h"
#include "subbus.h"

#define LIBRARY_SUB SB_SYSCON104
#define SUBBUS_VERSION 0x400 /* subbus version 4.00 QNX6 */

//---------------------------------------------------------------------
//Subbus Features:
//---------------------------------------------------------------------
#define SBF_SIC 1 // SIC Functions (SICFUNC)
#define SBF_LG_RAM 2 // Large NVRAM (SYSCON)
#define SBF_HW_CNTS 4 // Hardware rst & pwr Counters (SIC)
#define SBF_WD 8 // Watchdog functions (always)
#define SBF_SET_FAIL 0x10 // Set failure lamp (Comes w/ SICFUNC)
#define SBF_READ_FAIL 0x20 // Read failure lamps (SYSCON)
#define SBF_READ_SW 0x40 // Read Switches (was SICFUNC, now indep.)
#define SBF_NVRAM 0x80 // Any NVRAM at all!
#define SBF_CMDSTROBE 0x100 // Cmdstrobe Function


#if LIBRARY_SUB == 1
  #define PCICC 1
  #define SIC 0
  #define SYSCON 0
  #define SICFUNC 0
  #define SC104 0
  #define READSWITCH 0
  #define NVRAM 0
#endif

#if LIBRARY_SUB == 2
  #define PCICC 1
  #define SIC 1
  #define SYSCON 0
  #define SICFUNC 1
  #define SC104 0
  #define READSWITCH 1
  #define NVRAM 1
#endif

#if LIBRARY_SUB == 3
  #define PCICC 0
  #define SIC 0
  #define SYSCON 1
  #define SICFUNC 1
  #define SC104 0
  #define READSWITCH 1
  #define NVRAM 1
#endif

#if LIBRARY_SUB == 4
  #define PCICC 0
  #define SIC 0
  #define SYSCON 1
  #define SICFUNC 0
  #define SC104 1
  #define READSWITCH 1
  #define NVRAM 0
#endif

#if PCICC
  #define SC_SB_RESET 0x310
  #define SC_SB_LOWA 0x308
  #define SC_SB_HIGHA 0x30C
  #define SC_SB_LOWB 0x309
  #define SC_SB_HIGHB 0x30D
  #define SC_SB_LOWC 0x30A
  #define SC_SB_HIGHC 0x30E
  #define SC_SB_LOWCTRL 0x30B
  #define SC_SB_HIGHCTRL 0x30F
  #define SC_SB_CONFIG 0x0C1C0
  #define SC_CMDENBL 0x311
  #define SC_DISARM 0x318
  #define SC_TICK 0x319
  #define WAIT_COUNT 10
#endif

#if SIC
  #define SC_RES_CNT 0x31A
  #define SC_PWR_CNT 0x31B
  #define SC_RAMADDR 0x31C
  #define SC_NMI_ENABLE 0x31C
  #define SC_NMIE_VAL 0x20
  #define SC_RAMDATA 0x31D
  #define SC_SWITCHES 0x31E
  #define SC_LAMP 0x31F
#endif

#if SYSCON
  #define SC_SB_RESET 0x310
  #define SC_SB_LOWA 0x308
  #define SC_SB_LOWB 0x30A
  #define SC_SB_LOWC 0x30C
  #define SC_SB_LOWCTRL 0x30E
  #define SC_SB_HIGHA 0x309
  #define SC_SB_HIGHB 0x30B
  #define SC_SB_HIGHC 0x30D
  #define SC_SB_HIGHCTRL 0x30F
  #define SC_SB_CONFIG 0x0C1C0
  #define SC_CMDENBL 0x318
  #define SC_DISARM 0x311
  #define SC_TICK 0x319
  #define SC_LAMP 0x317
	#if SC104
    #define SC_SWITCHES 0x316
    #define WAIT_COUNT 5
    #define SET_FAIL 1
	#else
    #define SC_RAMADDR 0x31A
    #define SC_RAMDATA 0x31D
    #define SC_NMI_ENABLE 0x31C
    #define SC_NMIE_VAL 1 
    #define WAIT_COUNT 1
    #define SC_SWITCHES 0x31C
    #define LG_RAM 1
	#endif
#endif

#if SICFUNC
  #define TICKFAIL 6 // novram addr for tick fail info
  #ifndef SET_FAIL
    #define SET_FAIL 1
  #endif
#endif

//----------------------------------------------------------------
// SC104 is the first unit without any NVRAM
//----------------------------------------------------------------
#ifndef SET_FAIL
  #define SET_FAIL 0
#endif
#ifndef LG_RAM
  #define LG_RAM 0
#endif

#define SUBBUS_FEATURES (SBF_WD | (SIC*SBF_HW_CNTS) | (SET_FAIL*SBF_SET_FAIL) | \
 (LG_RAM*SBF_LG_RAM) | (SYSCON*(SBF_READ_FAIL|SBF_CMDSTROBE)) | \
 (SICFUNC*SBF_SIC) | (READSWITCH*SBF_READ_SW) | (NVRAM*SBF_NVRAM))

extern void process_IRQs( char *t );
extern int int_attach(int rcvid, subbusd_req_t *req);
extern int int_detach(int rcvid, subbusd_req_t *req);
extern int service_expint( message_context_t * ctp, int code,
                     unsigned flags, void * handle );
extern void expint_init( int coid, short code, int value );
extern int service_expint( message_context_t * ctp, int code,
                     unsigned flags, void * handle );

#endif

