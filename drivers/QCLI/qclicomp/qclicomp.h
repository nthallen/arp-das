#ifndef QCLICOMP_H_INCLUDED
#define QCLICOMP_H_INCLUDED
#include <math.h>
#include "err.h"
#include "csm.h"
#include "ptg_gen.h"

typedef struct {
  double samples;
  int naverage;
} RateDef, *RateDefP;

/* These are digitizer type codes */
#define DIG_CPCI14 0
#define DIG_CS210 1
#define DIG_SSP 2
/* These are QCLI configuration codes */
#define QCLI_CFG_STANDARD 0
#define QCLI_CFG_STANDARDxP33 1
#define QCLI_CFG_FASTRAMPx10 2
#define QCLI_CFG_FASTRAMP 3
#define QCLI_CFG_FASTRAMPx25 4
#define QCLI_CFG_STANDARDx2P41 5
#define QCLI_CFG_MAX 5

extern RateDefP NewRateDefPtr( double samples, int navg,
  int specd, int digitizer, CoordPtr pos );

typedef long *longP;
extern longP Set_rval( longP rvals, int index, double val );
extern long PickRes( int npts, longP lp );

#define N_DTOAS 3
typedef struct {
  double value[N_DTOAS];
  short bits[N_DTOAS];
  int n_used;
} WaveDtoA, *WaveDtoAP;
extern WaveDtoAP new_wavedtoa( void );
extern int alloc_dtoa( WaveDtoAP wdp, double delta, int qclicfg, CoordPtr pos );
extern double dtoa_value( WaveDtoAP wdp, int index );
extern short dtoa_bits( WaveDtoAP wdb, int index );
extern void set_dac3_bits( WaveDtoAP wdp, int bits, CoordPtr pos );
extern void set_dac3_value( WaveDtoAP wdp, double value, CoordPtr pos );

extern long round_to_step( double time, long step );
#define NULLRATE() NewRateDefPtr(0.,1,0,DIG_CPCI14,NoPosition)
#define PICKRATE(x,y) (x->samples>0?x:y)
#define usecs(x) ((long)floor((x)*1e6+.5))
#define DIV_UP(x,y) (((x)+(y)-1)/(y))
#define TZI_DEFAULT 10000
#define N_STEPS_MAX 1000
#define FIRSTNONNEG(x,y) ((x)>=0?(x):(y))
#define NEGONE() (-1.)

#define SW_TRIGGER    0x0001
#define SW_RAMP_IN    0x0002
#define SW_RAMP_RESET 0x0004
#define SW_DAC0_OUT   0x0008
#define SW_RAMP_OUT   0x0010
#define SW_DAC2_OUT   0x0020
#define SW_DAC3_OUT   0x0040
#define SW_NEG_OUT    0x0080
#define SW_POS_OUT    0x0100
#define SW_HSDAC_OUT  0x0200
#define SW_LASER_ON   SW_POS_OUT
#define WV_RAMP_ON    (SW_RAMP_IN|SW_RAMP_OUT|SW_LASER_ON)
#define WV_RAMP_OFF   (SW_RAMP_IN|SW_RAMP_RESET|SW_RAMP_OUT|SW_LASER_ON)
#define WV_RAMP_RESET (SW_RAMP_RESET|SW_LASER_ON)
#define WV_RAMP_OFF_T (WV_RAMP_OFF|SW_TRIGGER)

/* #define SW_RAMP_OFF_LASER_ON (SW_RAMP_OFF|SW_LASER_ON) */
/* #define SW_RAMP_ON_LASER_ON  (SW_RAMP_ON|SW_LASER_ON) */
/* #define SW_RAMP_OFF_LASER_ON_T (SW_RAMP_OFF_LASER_ON|SW_TRIGGER) */

extern void ptg_output_short( PTG_OUTPUT_FILE file, short value );
extern void ptg_output_word( PTG_OUTPUT_FILE file, unsigned short value, int count );
extern void ptg_output_time( PTG_OUTPUT_FILE file );
extern void ptg_output_name( PTG_OUTPUT_FILE file, const char *name );
#define ptg_output_hex( f, v, c ) ptg_output_word(f,(unsigned short)v,c)

/* New conversions for new op amp configuration 040423 */
/* #define AMPS_PER_BIT -7.6294e-5
   #define AMPS_BIT_OFFSET 32768.
   #define APS_PER_BIT 3.80625e-3
   #define APS_BIT_OFFSET 0
*/
#define AMPS_PER_BIT 3.8147e-5
#define AMPS_BIT_OFFSET 0
#define APS_PER_BIT -7.6294e-3
#define APS_BIT_OFFSET 32768.
#define MIN_DAC_BITS 0
#define MAX_DAC_BITS 65535L
  
unsigned short amps_to_bits( double amps, int qclicfg, CoordPtr pos );
unsigned short aps_to_bits( double aps, int qclicfg, CoordPtr pos );
#define ICOS_WAVEFORM_CODE 0x3331
#define RINGDOWN_WAVEFORM_CODE 0x3332
PTGNode RingdownPTG( double Istart, double Istop, double Istep, int ProgLen,
  int Ncoadd, int qclicfg, CoordPtr pos );

extern void InitCol(void);
extern void OutputLine(FILE *f, char *s);
extern void CondMatNL(FILE *f);

#endif
