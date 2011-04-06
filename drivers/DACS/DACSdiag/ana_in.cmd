&command
  : AI Set Gain %d(Enter Channel Address) &ai_gain * {
      if ( $4 < 0xC00 || $4 > 0xDFE || ($4 & 1) )
	nl_error(2, "Invalid AI Channel: 0x%04X", $4);
      else sbwr($4, $5);
    }
  : AI Fix Row %d(Enter Row Number) * {
      if ( $4 < 0 || $4 > 0x3F )
	nl_error(2, "Requested row out of range" );
      else sbwr( 0xC01, 0x40 | $4 );
    }
  : AI Stop Engine * { sbwr( 0xC01, 0x80 ); }
  : AI Cycle Engine * { sbwr( 0xC01, 0 ); }
  : AI Extra Settling * { sbwr( 0xC01, 0x200 ); }
  : AI Reset * { sbwr( 0xC01, 0x400 ); }
  : AI Increment Row by %d(Enter Row Increment) * {
      if ( $5 < 0 || $5 > 0x3F )
        nl_error(2, "Requested row out of range" );
      else sbwr( 0xC01, 0x100 | $5 );
    }
  : AI Double Convert Always * { sbwr( 0xC01, 0x1800 ); }
  : AI Double Convert Row Zero * { sbwr( 0xC01, 0x0800 ); }
  ;
&ai_gain <unsigned short>
  : Hi-Z { $0 = 1; }
  : Null { $0 = 2; }
  : 1 { $0 = 0x14; }
  : 2 { $0 = 0x1C; }
  : 0.768 { $0 = 0x18; }
  : 0.384 { $0 = 0x10; }
  : 0.192 { $0 = 0x08; }
  ;
