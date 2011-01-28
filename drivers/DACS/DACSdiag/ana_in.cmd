&command
  : AI Set Gain %d(Enter Channel Address) &ai_gain * {
      if ( $4 < 0xC00 || $4 > 0xDFE || ($4 & 1) )
	msg(2, "Invalid AI Channel: 0x%04X", $4);
      else sbwr($4, $5);
    }
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
