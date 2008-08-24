/* scdiag.h defines functions and values pertinent to syscon
   diagnostics.
   Written January 22, 1990
*/
extern unsigned int inpw(unsigned int);
extern void outpw(unsigned int, unsigned int);
extern void diag_name(char *name);
extern void diag_status(unsigned char attr, char *fmt, ...);
extern int disp_addrs(int,int,int,int,int *,int, int *, int);

/* Here are the arguments to the diagnostic functions: */
#define AUTO_MODE 1
#define MAN_MODE 0
#define CON_MODE 2
/* And the return values */
#define SCD_PASS 1
#define SCD_FAIL 0

/* And here are the diagnostic functions themselves */
typedef struct {
  char *name;  /* name for the display */
  char mmm[4]; /* three-letter mnemonic */
  int (*func)(int); /* the function itself */
} diagdef;

int addr_debug(int mode);
int H2O(int mode);
int AtoD0(int mode);
int AtoD1(int mode);
int AtoD2(int mode);
int AtoD3(int mode);
int AtoD4(int mode);
int AtoD5(int mode);
int AtoD6(int mode);
int AtoD7(int mode);
#ifdef card
int DtoAtest(int mode);
#endif
#ifdef ana104
int DtoAtest0(int mode);
int DtoAtest1(int mode);
int DtoAtest2(int mode);
int DtoAtest3(int mode);
int DtoAtest4(int mode);
int DtoAtest5(int mode);
int DtoAtest6(int mode);
int DtoAtest7(int mode);
#endif
int subbus_low(int mode);
int subbus_high(int mode);
int subbus_word(int mode);
int nv_ram_test(int mode);
int pattern_test(int mode);
int cmdenbl_test(int mode);
int reboot_test(int mode);
int input_port(int mode);
int nmi_test(int mode);

