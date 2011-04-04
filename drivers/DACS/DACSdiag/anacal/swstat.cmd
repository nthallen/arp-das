%{
  #include "swstat.h"
  swstat_t SWData;
%}

%INTERFACE <SWData:DG/data>

&command
  : Fail Light &on_off * { set_failure($3); }
  : &SWTM * { if_SWData.Turf(); }
  ;
&SWTM
  : SW Status &swstat {
      SWData.SWStat = $3;
    }
  ;
&swstat <int>
  : Set %d { $0 = $2; }
# : Indexer Test 0 { $0 = SWS_IDX_TEST0; }
  : AO Ramp { $0 = SWS_AO_RAMP; }
  : AO Idle { $0 = SWS_AO_IDLE; }
  ;
