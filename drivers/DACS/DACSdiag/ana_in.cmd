&command
  : AI %d (row) %d(col) %d(bank) config %d *
    { unsigned short addr = 0xC00 + $2*32 + $4*16 + $3*2;
      msg( 0, "Writing %d to %04X", $6, addr );
      sbwr(addr, $6);
    }
  ;
