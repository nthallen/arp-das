%{

#ifdef SERVER
  #include "hsatod.h"
  #define QCLI1_ICOS (HSAD_OPT_A|HSAD_OPT_B|HSAD_OPT_C|HSAD_TRIG_3|HSAD_TRIG_RISING)
  #define QCLI2_ICOS (HSAD_OPT_A|HSAD_OPT_B|HSAD_TRIG_3|HSAD_TRIG_RISING)
  #define QCLI2_RINGDOWN QCLI2_ICOS
  #define QCLI3_ICOS (HSAD_OPT_A|HSAD_OPT_B|HSAD_TRIG_3|HSAD_TRIG_RISING)
  #define QCLI3_RINGDOWN QCLI3_ICOS

  hsatod_setup_t ssp1_setup;
  extern IOFUNC_ATTR_T *if_SSP1;

  struct {
    hsatod_setup_t *setup;
    IOFUNC_ATTR_T **intf;
  } ssp_bd[3] = {
    { &ssp1_setup, &if_SSP1 },
  };

#endif

%}

%INTERFACE <SSP1>

&command
  : &SSP Start * {
        hsatod_setup_t *setup = ssp_bd[$1].setup;
        if (setup->NAvg > 0) {
	  cis_turf( *ssp_bd[$1].intf,
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
  : &SSP Stop * { cis_turf( *ssp_bd[$1].intf, "DA" ); }
  : &SSP Reset * { cis_turf( *ssp_bd[$1].intf, "XR" ); }
  : &SSP Exit * { cis_turf( *ssp_bd[$1].intf, "XX" ); }
  : &SSP Set NSample %ld (Number of Samples) * {
        ssp_bd[$1].setup->NSample = $4;
      }
  : &SSP Set FSample %ld (Raw Sample Frequency) * {
        ssp_bd[$1].setup->FSample = $4;
      }
  : &SSP Set NAvg %ld (Number of Consecutive Samples to Average) * {
        ssp_bd[$1].setup->NAvg = $4;
      }
  : &SSP Set NCoadd %d (Number of Scans to Coadd) * {
        ssp_bd[$1].setup->NCoadd = $4;
      }
  : &SSP Set Trigger &Trigger * {
          nl_error( 0, "Set trigger command: '%s'", $4 );
          cis_turf( *ssp_bd[$1].intf, $4 );
      }
  : &SSP Set Trigger Level %d (Enter Trigger Level) &TrigPolarity * {
          cis_turf( *ssp_bd[$1].intf, "%s:%d\n", $6, $5 );
      }
  : &SSP Logging &LogEnable * { cis_turf( *ssp_bd[$1].intf, $3 ); }
  ;

# &SSP returns an index into ssp_bd[]
&SSP <int>
  : SSP { $0 = 0; }
  ;

&Trigger <char *>
  : Source &TrigSource { $0 = $2; }
  : Auto { $0 = "AE\n"; }
  : NoAuto { $0 = "AD\n"; }
  ;

# TrigSource returns the appropriate trigger source string,
# which will be sent to sspdrv.
&TrigSource <char *>
  : Channel 0 { $0 = "TS:0\n"; }
  : Channel 1 { $0 = "TS:1\n"; }
  : Channel 2 { $0 = "TS:2\n"; }
  : External { $0 = "TS:3\n"; }
  ;
&TrigPolarity <char *>
  : Rising { $0 = "TU"; }
  : Falling { $0 = "TD"; }
  ;
&LogEnable <char *>
  : Enable { $0 = "LE\n"; }
  : Disable { $0 = "LD\n"; }
  ;


&command
  : Select QCLI Waveform &QCLI1_Wave * {
      *ssp_bd[0].setup = QCLI1_Waves[$4];
      cis_turf( if_QCLI1, "SW:%d\n", $4 );
    }
  ;
