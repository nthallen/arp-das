&command
  : Fail Light &on_off * { set_failure($3); }
  : &SWTM * {
      if ( SWS_id == 0 )
	SWS_id = Col_send_init( "SWData", &SWData, sizeof(SWData), 0 );
      Col_send(SWS_id);
    }
  ;
&SWTM
  : SW Status &swstat {
      SWData.SWStat = $3;
    }
  ;
&swstat <int>
  : Set %d { $0 = $2; }
  : Indexer Test 0 { $0 = SWS_IDX_TEST0; }
  ;
