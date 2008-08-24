/* suite.c contains test functions for syscon diagnostics.
   Written January 22, 1990
*/
/*
   ====================================================================== 
   Functions return an int SCD_PASS or SCD_FAIL.
   The argument to a function is an int AUTO_MODE, MAN_MODE or CON_MODE.
   For AUTO_MODE and CON_MODE, nodelay(stdsrc,TRUE) is in effect.
   For MAN_MODE nodelay(stdscr,FALSE) is in effect. Please leave it like
   that on return.
   ======================================================================
*/

#include <curses.h>
#undef getch
#ifdef DOS
#include <dos.h>
#include "reslib.h"
#endif
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <cfg.h>
#include "syscon.h"
#include "scdiag.h"
#include "attribut.h"
#include "define.h"
#include "subbus.h"

unsigned int sb_data[] = {0, 0xFFFF, 0x00FF, 0x0055, 0xFE01, 0xFD02,
		 0xFB04, 0xF708, 0xEF10, 0xDF20, 0xBF40, 0x7F80};
#define N_WORDS (sizeof(sb_data)/sizeof(int))

int is_syscon104( void ) {
  static isit = -1;
  
  if ( isit == -1 ) {
	unsigned short pattern=0x5555, readback;
	outp(SC_SB_RESET, 0);
	outpw(SC_SB_CONTROL, SC_SB_CONFIG);
	outpw( SC_SB_PORTB, pattern );
	readback = inpw( SC_SB_PORTB );
	if ( readback == 0x4B01 ) isit = 1;
	else isit = 0;
  }
  return isit;
}

unsigned short pattern_register(void) {
  return is_syscon104() ? SC_SB_PORTA : SC_SB_PORTB;
}

int subbus_report(int i, unsigned p, int size) {
  if (i != N_WORDS) {
    diag_status(ATTR_FAIL, "Failure on pattern %0*X", size, p);
    return(SCD_FAIL);
  } else {
    diag_status(ATTR_PASS, "PASSED");
    return(SCD_PASS);
  }
}

/* Subbus diagnostics */

int load_sublib(void) {
  int c;  
  c = load_subbus();
  
  if (c == 0) {
   diag_status(ATTR_FAIL,"No subbus library installed");
   return(0);
  }
  else if (c < 0 || c > 4) {
   diag_status(ATTR_FAIL,"Unknown subbus library installed");
   return(0);
  }    
  return(1);
}    
    
int addr_debug(int mode) {
    int cmds[20]={ESCAPE,CR,CTRLR,CTRLW,M,m,B,b,R,r,W,w,A,a,D,d,PLUS,MINUS,KEY_UP,KEY_DOWN};
    if (load_sublib())
       if (mode!=MAN_MODE)
          diag_status(ATTR_PASS,"Subbus Library is Installed");
       else {
          diag_status(ATTR_PASS,"In Manual Mode"); refresh();
          disp_addrs(0,0,HEX,WORDRES,cmds,20,0,0);
          diag_status(ATTR_PASS,"Manual Debug Completed");
       }
    else return(SCD_FAIL);
    return(SCD_PASS);
}

#ifdef card
int dtoa(int addr1, int addr2, int addr3, int addr4, int mode) {
    int cmds[23]={ESCAPE,CR,CTRLL,L,l,CTRLW,W,w,M,m,B,b,S,s,D,d,KEY_UP,KEY_DOWN,KEY_RIGHT,KEY_LEFT,PLUS,MINUS,CTRLC};
    int addrs[4];
    addrs[0]=addr1; addrs[1]=addr2; addrs[2]=addr3; addrs[3]=addr4;    
    if (load_sublib())
       if (mode!=MAN_MODE)
          diag_status(ATTR_PASS,"Subbus Library is Installed");
       else {
          diag_status(ATTR_PASS,"In Manual Mode"); refresh();
          disp_addrs(addr1,addr1,HEX,WORDRES,cmds,23,addrs,4);
          diag_status(ATTR_PASS,"Manual Debug Completed");
       }
    else return(SCD_FAIL);   
    return(SCD_PASS);        
}   
#endif

#if defined(card) || defined(ana104)
int subbus_debug(int from, int to) {
  int i, fail=0, failall=0, data;
  char stat[40];

  if (load_sublib()) {
     for (i=from;i<=to;i++)
       if (!read_ack(0,i,(unsigned far *)(&data))) {
         fail=i; failall++;
       }
     if (failall==(to-from+1)) {
        diag_status(ATTR_FAIL,"Card Not Present");
        return(0);
     }
     if (fail) {
       sprintf(stat,"No Ack at 0x%X",fail);
       diag_status(ATTR_FAIL,stat);
       return(0);
     }
     if (read_ack(0,0,(unsigned far *)(&data))) {
        diag_status(ATTR_WARN,"Permanent Ack Detected");
        return(1);
     }
     diag_status(ATTR_PASS,"Ack for All Addresses");
     return(1);  
  }
  return 0;
}

int card_debug(int from, int to, int mode) {
     int cmds[5]={ESCAPE,M,m,B,b};    
     if (subbus_debug(from,to)) {
         if (mode==MAN_MODE) {
         diag_status(ATTR_PASS,"In Manual Mode"); refresh();
         disp_addrs(from,to,HEX,WORDRES,cmds,5,0,0);
         diag_status(ATTR_PASS,"Manual Debug Completed");
        }
	 return(SCD_PASS);
     }
     else return(SCD_FAIL);    
}
#endif

#ifdef card
int DtoAtest(int mode) {
    return(dtoa(123,456,789,101,mode));
}

int H2O(int mode) {
    return(card_debug(H2O_BEG,H2O_END,mode));
}

int AtoD0(int mode) {
    return(card_debug(AtoD0_BEG,AtoD0_END,mode));
}
 
int AtoD1(int mode) {
    return(card_debug(AtoD1_BEG,AtoD1_END,mode));
}    

int AtoD2(int mode) {
    return(card_debug(AtoD2_BEG,AtoD2_END,mode));
}    

int AtoD3(int mode) {
    return(card_debug(AtoD3_BEG,AtoD3_END,mode));
}

int AtoD4(int mode) {
    return(card_debug(AtoD4_BEG,AtoD4_END,mode));
}

int AtoD5(int mode) {
    return(card_debug(AtoD5_BEG,AtoD5_END,mode));
}

int AtoD6(int mode) {
    return(card_debug(AtoD6_BEG,AtoD6_END,mode));
}

int AtoD7(int mode) {
    return(card_debug(AtoD7_BEG,AtoD7_END,mode));
}

#endif

#ifdef ana104

int dtoa(int addr1,int mode) {
    int cmds[23]={ESCAPE,CR,CTRLL,L,l,CTRLW,W,w,M,m,B,b,S,s,D,d,KEY_UP,KEY_DOWN,KEY_RIGHT,KEY_LEFT,PLUS,MINUS,CTRLC};
    int addrs[9];
    int i;
    for (i=0;i<9;i++) addrs[i] = addr1 + (i*2);
    if (load_sublib())
       if (mode!=MAN_MODE)
          diag_status(ATTR_PASS,"Subbus Library is Installed");
       else {
          diag_status(ATTR_PASS,"In Manual Mode"); refresh();
          disp_addrs(addr1,addr1,HEX,WORDRES,cmds,23,addrs,8);
          diag_status(ATTR_PASS,"Manual Debug Completed");
       }
    else return(SCD_FAIL);   
    return(SCD_PASS);        
}   

int DtoAtest0(int mode) {
    return(dtoa(DtoA0_104_BEG,mode));
}
int DtoAtest1(int mode) {
    return(dtoa(DtoA1_104_BEG,mode));
}
int DtoAtest2(int mode) {
    return(dtoa(DtoA2_104_BEG,mode));
}
int DtoAtest3(int mode) {
    return(dtoa(DtoA3_104_BEG,mode));
}
int DtoAtest4(int mode) {
    return(dtoa(DtoA4_104_BEG,mode));
}
int DtoAtest5(int mode) {
    return(dtoa(DtoA5_104_BEG,mode));
}
int DtoAtest6(int mode) {
    return(dtoa(DtoA6_104_BEG,mode));
}
int DtoAtest7(int mode) {
    return(dtoa(DtoA7_104_BEG,mode));
}

int AtoD0(int mode) {
    return(card_debug(AtoD0_104_BEG,AtoD0_104_END,mode));
}
 
int AtoD1(int mode) {
    return(card_debug(AtoD1_104_BEG,AtoD1_104_END,mode));
}    

int AtoD2(int mode) {
    return(card_debug(AtoD2_104_BEG,AtoD2_104_END,mode));
}    

int AtoD3(int mode) {
    return(card_debug(AtoD3_104_BEG,AtoD3_104_END,mode));
}

int AtoD4(int mode) {
    return(card_debug(AtoD4_104_BEG,AtoD4_104_END,mode));
}

int AtoD5(int mode) {
    return(card_debug(AtoD5_104_BEG,AtoD5_104_END,mode));
}

int AtoD6(int mode) {
    return(card_debug(AtoD6_104_BEG,AtoD6_104_END,mode));
}

int AtoD7(int mode) {
    return(card_debug(AtoD7_104_BEG,AtoD7_104_END,mode));
}

#endif

#ifdef syscon
/* Returns TRUE if the rd+wr bit goes low */
int wait_for_104(void) {
  int i;
  for ( i = 0; i < 10; i++ ) {
	if ( !(inpw(SC_SB_PORTC) & 0x800) )
	  return 1;
  }
  return 0;
}

int subbus_low(int mode) {
  int i;
  unsigned char pat;
  unsigned short patreg;
  int is104 = is_syscon104();

  patreg = pattern_register();
  for (i = 0; i < N_WORDS; i++) {
    pat = sb_data[i] & 0xFF;
    outp( patreg, pat);
	if ( is104 && ! wait_for_104() ) break;
    if ( inp( patreg ) != pat) break;
    pat = (sb_data[i] >> 8) & 0xFF;
    outp( patreg, pat);
	if ( is104 && ! wait_for_104() ) break;
    if ( inp( patreg ) != pat) break;
  }
  return subbus_report(i, pat, 2);
}

/* Subbus diagnostics */
int subbus_high(int mode) {
  int i;
  unsigned char pat;
  unsigned short patreg;
  int is104 = is_syscon104();

  patreg = pattern_register();
  for (i = 0; i < N_WORDS; i++) {
    pat = sb_data[i] & 0xFF;
    outp( patreg+1, pat);
	if ( is104 && ! wait_for_104() ) break;
    if ( inp( patreg+1 ) != pat ) break;
    pat = (sb_data[i] >> 8) & 0xFF;
    outp( patreg+1, pat );
	if ( is104 && ! wait_for_104() ) break;
    if (inp( patreg+1 ) != pat ) break;
  }
  return subbus_report(i, pat, 2);
}

/* Subbus diagnostics */
int subbus_word(int mode) {
  int i;
  unsigned int pat, rpat;
  unsigned char ph, pl;
  unsigned short patreg;
  int is104 = is_syscon104();

  patreg = pattern_register();
  for (i = 0; i < N_WORDS; i++) {
    pat = sb_data[i];
    pl = pat & 0xFF;
    ph = ( pat >> 8 ) & 0xFF;
    rpat = ( pl << 8 ) | ph;
    outpw( patreg, pat );
	if ( is104 && ! wait_for_104() ) break;
    if (inp( patreg ) != pl || inp( patreg+1 ) != ph) break;
	if ( ! is_syscon104() ) {
	  outp( patreg, ph );
	  outp( patreg+1, pl );
	  if (inpw( patreg ) != rpat) break;
	}
  }
  return(subbus_report(i, pat, 4));
}

/* NV-RAM
   Read RAM to see if it contains a recognized test pattern.
   Write test patterns to the entire RAM and read them back.
   Allow selectable test patterns to be written prior to a
   power-down to verify the ability to hold data across a power
   outage.
   Patterns: address, 0, FF, 55, AA, 0F, F0
*/
unsigned char rampats[] = {0, 0xFF, 0x55, 0xAA, 0x0F, 0xF0};
#define N_RAMPATS ((int)sizeof(rampats))

void w_nvram(unsigned int addr, unsigned char val) {
  outpw(SC_NVADDR, addr);
  outp(SC_NVRAM, val);
}

unsigned char r_nvram(unsigned int addr) {
  outpw(SC_NVADDR, addr);
  return((unsigned char)(inp(SC_NVRAM) & 0xFF));
}

void ram_write(int pattern) {
  unsigned char patchar;
  int i;

  assert(pattern >= 0 && pattern <= N_RAMPATS);
  w_nvram(0, (unsigned char)(pattern+RAM_PATTERNS));
  if (pattern > 0) {
    patchar = rampats[pattern-1];
    for (i = 1; i < 512; i++) w_nvram(i, patchar);
  } else {
    for (i = 1; i < 512; i++)
      w_nvram(i, (unsigned char) (i < 256 ? i : 511-i));
  }
}

int ram_verify(int pattern) {
  unsigned char patchar;
  int i;

  assert(pattern >= 0 && pattern <= N_RAMPATS);
  if (r_nvram(0) != pattern+RAM_PATTERNS) return(SCD_FAIL);
  if (pattern > 0) {
    patchar = rampats[pattern-1];
    for (i = 1; i < 512; i++) if (r_nvram(i) != patchar) return(SCD_FAIL);
  } else {
    for (i = 1; i < 512; i++)
      if (r_nvram(i) != (i < 256 ? i : 511-i)) return(SCD_FAIL);
  }
  return(SCD_PASS);
}
static char *stat_txt[] = { "READY",
    "INIT",
    "RUNNING",
    "PFAIL_DET",
    "PFAIL_OK",
    "FLT_OVER",
    "DUMPED",
    "PWR_CYC_REQ",
    "FAILURE",
    "EARLY",
    "FLT_OVER2"
};

/* non-volatile ram tests: the pattern check */
int nv_ram_test(int mode) {
  int n;

  if ( is_syscon104() ) {
    diag_status(ATTR_PASS, "Not Applicable");
    return(SCD_PASS);
  }
  n = r_nvram(0);
  switch (n) {
    case RAM_READY:
    case RAM_INIT:
    case RAM_RUNNING:
    case RAM_PFAIL_DET:
    case RAM_PFAIL_OK:
    case RAM_FLT_OVER:
    case RAM_DUMPED:
    case RAM_PWR_CYC_REQ:
    case RAM_FAILURE:
    case RAM_EARLY:
    case RAM_FLT_OVER2:
      diag_status(ATTR_PASS, "Critical Status: %s", stat_txt[n]);
      break;
    case RAM_SYSRESET: /* system reset test */
      diag_status(ATTR_PASS, "System Reset Test");
      w_nvram(0, RAM_INACTIVE);
      break;
    case RAM_PFDIAG: /* Power Fail test */
      n = (r_nvram(0x1FE) << 8) + r_nvram(0x1FF);
      diag_status((unsigned char)(n != 0 ? ATTR_PASS : ATTR_FAIL),
		"Power Fail Test: %d iterations", n);
      w_nvram(0, RAM_INACTIVE);
      break;
    case RAM_PATTERNS: /* Test patterns */
    case RAM_PATTERNS + 1:
    case RAM_PATTERNS + 2:
    case RAM_PATTERNS + 3:
    case RAM_PATTERNS + 4:
    case RAM_PATTERNS + 5:
    case RAM_PATTERNS + 6:
      if (ram_verify(n - RAM_PATTERNS)) {
	diag_status(ATTR_PASS,
		"Pattern %d present before testing", n - RAM_PATTERNS);
        break;
      }
    case RAM_INACTIVE:   /* Reserved for no particular value */
    default:	/* fall through for no pattern */
      diag_status(ATTR_WARN,
	        "No recognized pattern present before testing");
      break;
  }
  return(SCD_PASS);
}


int pattern_test(int mode) {
  int i, j;
  unsigned char *ramsav;
  static int manpat = 0;

  if ( is_syscon104() ) {
    diag_status(ATTR_PASS, "Not Applicable");
    return(SCD_PASS);
  }
  if (mode == MAN_MODE) {
    i = manpat;
    if (++manpat >= N_RAMPATS) manpat = 0;
    ram_write(i);
    if (ram_verify(i)) {
      diag_status(ATTR_PASS, "RAM pattern %d written and verified", i);
      return(SCD_PASS);
    }
  } else {
    ramsav = malloc(512);
    if (ramsav == NULL) {
      diag_status(ATTR_FAIL, "Memory Allocation Failed");
      return(SCD_FAIL);
    }
    for (j = 0; j < 512; j++) ramsav[j] = r_nvram(j);
    for (i = 0; i <= N_RAMPATS; i++) {
      ram_write(i);
      if (!ram_verify(i)) break;
    }
    for (j = 0; j < 512; j++) w_nvram(j, ramsav[j]);
    free(ramsav);
  }
  if (i < N_RAMPATS) {
    diag_status(ATTR_FAIL, "RAM pattern %d failed to verify", i);
    return(SCD_FAIL);
  }
  diag_status(ATTR_PASS, "All RAM patterns written and verified");
  return(SCD_PASS);
}


int cmdenbl_test(int mode) {
  int n;

  outp(SC_CMDENBL, 1);
  outp(SC_TICK, 0);
  n = SC_CMDENBL_BCKPLN;
  outp(SC_DISARM, 0);
  if (n != 0) diag_status(ATTR_FAIL, "CMDENBL not observed");
  else {
    n = SC_CMDENBL_BCKPLN;
    if (n == 0) diag_status(ATTR_FAIL, "CMDENBL not cleared on disarm");
    else {
      diag_status(ATTR_PASS, "PASSED");
      return(SCD_PASS);
    }
  }
  return(SCD_FAIL);
}

int reboot_test(int mode) {
  int n;
  long int T;
  unsigned char ramsav;

  if (mode == MAN_MODE) {
    ramsav = r_nvram(0);
    w_nvram(0, RAM_SYSRESET); /* Write Reboot test value */
    outp(SC_CMDENBL, 1);
    outp(SC_TICK, 0);
    n = SC_CMDENBL_BCKPLN;
    if (n != 0) {
      diag_status(ATTR_FAIL, "CMDENBL not observed");
      outp(SC_DISARM, 0);
      n = SCD_FAIL;
    } else {
      T = time(NULL);
      do {
        n = SC_CMDENBL_BCKPLN;
        if (n != 0) break;
      } while (time(NULL) - T < 4);
      if (n == 0) {
        diag_status(ATTR_FAIL, "Timeout without any observed reset");
        n = SCD_FAIL;
      } else {
        diag_status(ATTR_PASS,
          "CMDENBL reset within %ld seconds.", time(NULL) - T + 1);
        n = SCD_PASS;
      }
    }
    w_nvram(0, ramsav); /* restore the critical value */
    return(n);
  } else return(SCD_PASS);
}

#define SWITCH_ON 223
#define SWITCH_OFF 220

void display_port(unsigned char p) {
  char buf[80];
  unsigned char mask = 0x80;
  int i;

  for (i = 0; mask != 0; mask >>= 1) {
    buf[i++] = ' ';
    buf[i++] = (p & mask) ? SWITCH_ON : SWITCH_OFF;
  }
  sprintf(&buf[i], "  (%c = on, %c = off)", SWITCH_ON, SWITCH_OFF);
  diag_status(ATTR_PASS, buf);
}

int input_port(int mode) {
  unsigned char old, new;
  unsigned short input_addr;
  
  input_addr = is_syscon104() ? 0x316 : SC_INPUT;
  old = inp( input_addr );
  display_port(old);
  if (mode == MAN_MODE) {
    refresh();
    noecho();
    raw();
    nodelay(stdscr, TRUE);
    while (wgetch(stdscr) == ERR) {
      new = inp( input_addr );
      if (new != old) {
    	display_port(old = new);
    	refresh();
      }
    }
  }
  return(SCD_PASS);
}

/*int nmi_test(int mode) {
  int i;
  extern volatile int nmi_seen;
  extern void set_nmi(void), reset_nmi(void);

  outp(SC_NMIENBL, 0);
  if (SC_LOWPOWER) {
    diag_status(ATTR_FAIL, "LOWPOWER Asserted");
    return(SCD_FAIL);
  }
  if (mode != MAN_MODE) {
    diag_status(ATTR_PASS, "LOWPOWER Clear: NMI not tested");
    return(SCD_PASS);
  }
  diag_status(ATTR_PASS, "LOWPOWER Clear: Push Low Power Test");
  refresh();
  while (!kbhit()) if (SC_LOWPOWER) break;
  if (kbhit()) {
    while (kbhit()) getch();
    diag_status(ATTR_FAIL, "LOWPOWER Not Observed");
    return(SCD_FAIL);
  }
  diag_status(ATTR_PASS, "LOWPOWER Set: Hit key to proceed to NMI Test");
  refresh();
  getch();
  set_nmi();
  outp(SC_NMIENBL, 1);
  for (i = 0; i < 1000; i++) if (nmi_seen) break;
  if (nmi_seen) {
    reset_nmi();
    diag_status(ATTR_FAIL, "Premature NMI");
    return(SCD_FAIL);
  }
  diag_status(ATTR_PASS, "Push Low Power Test");
  refresh();
  while (!kbhit()) if (nmi_seen || SC_LOWPOWER) break;
  reset_nmi();
  if (nmi_seen) {
    diag_status(ATTR_PASS, "NMI Intercepted");
    return(SCD_FAIL);
  } else if (SC_LOWPOWER)
    diag_status(ATTR_FAIL, "LOWPOWER Observed without NMI");
  else if (kbhit()) {
    while (kbhit()) getch();
    diag_status(ATTR_FAIL, "Neither LOWPOWER nor NMI seen");
  }
  return(SCD_FAIL);
}
*/

#endif
