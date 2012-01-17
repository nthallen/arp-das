%{
  #include "hsatod.h"
%}

&command
  : &QCLI &qclicommand0 * { $1->Turf( $2 ); }
  : &QCLI Write &DAC_Val to &DAC_Num * {
        $1->Turf( "D%d:%d\n", $5, $3 );
      }
  : &QCLI Set &QCLI_T_Param to &QCLI_T_val * {
        $1->Turf( "%s:%d\n", $3, $5 );
      }
# : &QCLI Set Heater to %lf (Enter Voltage) Volts * {
#     double dbits = $5 * 3252.5 - 749.8;
#     if ( dbits < 0 ) dbits = 0;
#     if ( dbits > 65535 ) {
#       nl_error( 2,
#         "DAC3 requested value out of range: %lf V = %lf bits",
#         $5, dbits );
#     } else {
#       unsigned short ibits = (unsigned short)dbits;
#       $1->Turf( "D3:%u\n", ibits );
#     }
#   }
  ;

&qclicommand0 <const char *>
  : Run { $0 = "RW\n"; }
  : Stop { $0 = "ST\n"; }
  : Clear Errors { $0 = "CE\n"; }
  : Exit { $0 = "QU\n"; }
  ;
&QCLI_T_Param <const char *>
  : Ton { $0 = "TN"; }
  : Toff { $0 = "TF"; }
  : Tpre { $0 = "TP"; }
  ;
&DAC_Val <short>
  : %d (Enter digital value) { $0 = $1; }
  ;
&DAC_Num <short>
  : DAC0 { $0 = 0; }
  : Ramp DAC { $0 = 1; }
  : DAC2 { $0 = 2; }
  : DAC3 { $0 = 3; }
  ;
&QCLI_T_val <unsigned short>
  : %d (Enter time in usecs) { $0 = $1; }
  ;

# SSP Commands

&command
  : &SSP Start * {
        hsatod_setup_t *setup = sspqcli_bd[$1].setup;
        if (setup->NAvg > 0) {
	  sspqcli_bd[$1].if_ssp->Turf(
	   "DA NF:%ld NS:%d NA:%d NC:%d NE:%d "
	   "T%c:0 TS:%d A%c EN\n",
	   setup->FSample, setup->NSample/setup->NAvg,
	   setup->NAvg, setup->NCoadd,
	   setup->Options & 7,
	   (setup->Options & HSAD_TRIG_RISING) ? 'U' : 'D',
	   (setup->Options & HSAD_TRIG_3)/HSAD_TRIG_1,
	   (setup->Options & HSAD_TRIG_AUTO) ? 'E' : 'D' );
        } else nl_error(2, "SSP %d NAvg out of range", $1+1);
      }
  : &SSP Stop * { sspqcli_bd[$1].if_ssp->Turf( "DA" ); }
  : &SSP Reset * { sspqcli_bd[$1].if_ssp->Turf( "XR" ); }
  : &SSP Exit * { sspqcli_bd[$1].if_ssp->Turf( "XX" ); }
  : &SSP Set NSample %ld (Number of Samples) * {
        sspqcli_bd[$1].setup->NSample = $4;
      }
  : &SSP Set FSample %ld (Raw Sample Frequency) * {
        sspqcli_bd[$1].setup->FSample = $4;
      }
  : &SSP Set NAvg %ld (Number of Consecutive Samples to Average) * {
        sspqcli_bd[$1].setup->NAvg = $4;
      }
  : &SSP Set NCoadd %d (Number of Scans to Coadd) * {
        sspqcli_bd[$1].setup->NCoadd = $4;
      }
  : &SSP Set Trigger &Trigger * {
          nl_error( 0, "Set trigger command: '%s'", $4 );
          sspqcli_bd[$1].if_ssp->Turf( $4 );
      }
  : &SSP Set Trigger Level %d (Enter Trigger Level) &TrigPolarity * {
          sspqcli_bd[$1].if_ssp->Turf( "%s:%d\n", $6, $5 );
      }
  : &SSP Logging &LogEnable * { sspqcli_bd[$1].if_ssp->Turf( $3 ); }
  ;

&Trigger <const char *>
  : Source &TrigSource { $0 = $2; }
  : Auto { $0 = "AE\n"; }
  : NoAuto { $0 = "AD\n"; }
  ;

# TrigSource returns the appropriate trigger source string,
# which will be sent to sspdrv.
&TrigSource <const char *>
  : Channel 0 { $0 = "TS:0\n"; }
  : Channel 1 { $0 = "TS:1\n"; }
  : Channel 2 { $0 = "TS:2\n"; }
  : External { $0 = "TS:3\n"; }
  ;
&TrigPolarity <const char *>
  : Rising { $0 = "TU"; }
  : Falling { $0 = "TD"; }
  ;
&LogEnable <const char *>
  : Enable { $0 = "LE\n"; }
  : Disable { $0 = "LD\n"; }
  ;

