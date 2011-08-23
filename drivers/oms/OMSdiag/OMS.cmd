%INTERFACE <oms>

&command
  : OMS &omscommand
  ;
&omscommand
  : Drive &omsaxis &omsdir &omsdistance * {
      if ( $3 ) {
	if_oms.Turf( "WA%cMR%ld;GO", $2, $3*$4 );
      } else {
	if_oms.Turf( "WA%cMA%ld;GO", $2, $4 );
      }
    }
  : Set &omsaxis Speed &omsspeed * {
      if_oms.Turf( "WA%cVL%ld", $2, $4 );
    }
  : Preset &omsaxis Position &omsdistance * {
      if_oms.Turf( "WA%cLP%ld", $2, $4 );
    }
  ;
&omsaxis <char>
  : 0 { $0 = 'X'; }
  : 1 { $0 = 'Y'; }
  : 2 { $0 = 'Z'; }
  : 3 { $0 = 'T'; }
  ;
&omsdir <int>
  : In { $0 = -1; }
  : Out { $0 = 1; }
  : To { $0 = 0; }
  ;
&omsdistance <long>
  : %ld (Enter distance in steps) { $0 = $1; }
  ;
&omsspeed <long>
  : %ld (Enter speed in steps/second) { $0 = $1; }
  ;
