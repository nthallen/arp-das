%INTERFACE <dccc>
&command
  : &dccc_cmd * { cis_turf(if_dccc, "D%d\n", $1); }
  ;
&dccc_cmd <int>
  : Command 0 On { $0 = 0; }
  : Command 0 Off { $0 = 1; }
  : Command 1 On { $0 = 2; }
  : Command 1 Off { $0 = 3; }
  : Command 2 On { $0 = 4; }
  : Command 2 Off { $0 = 5; }
  : Command 3 On { $0 = 6; }
  : Command 3 Off { $0 = 7; }
  : Command 4 On { $0 = 8; }
  : Command 4 Off { $0 = 9; }
  : Command 5 On { $0 = 10; }
  : Command 5 Off { $0 = 11; }
  : Command 6 On { $0 = 12; }
  : Command 6 Off { $0 = 13; }
  : Command 7 On { $0 = 14; }
  : Command 7 Off { $0 = 15; }
  : Command 8 On { $0 = 16; }
  : Command 8 Off { $0 = 17; }
  : Command 9 On { $0 = 18; }
  : Command 9 Off { $0 = 19; }
  : Command 10 On { $0 = 20; }
  : Command 10 Off { $0 = 21; }
  : Command 11 On { $0 = 22; }
  : Command 11 Off { $0 = 23; }
  : Command 12 On { $0 = 24; }
  : Command 12 Off { $0 = 25; }
  : Command 13 On { $0 = 26; }
  : Command 13 Off { $0 = 27; }
  : Command 14 On { $0 = 28; }
  : Command 14 Off { $0 = 29; }
  : Command 15 On { $0 = 30; }
  : Command 15 Off { $0 = 31; }
  : Command 16 On { $0 = 32; }
  : Command 16 Off { $0 = 33; }
  : Command 17 On { $0 = 34; }
  : Command 17 Off { $0 = 35; }
  : Command 18 On { $0 = 36; }
  : Command 18 Off { $0 = 37; }
  : Command 19 On { $0 = 38; }
  : Command 19 Off { $0 = 39; }
  : Command 20 On { $0 = 40; }
  : Command 20 Off { $0 = 41; }
  : Command 21 On { $0 = 42; }
  : Command 21 Off { $0 = 43; }
  : Command 22 On { $0 = 44; }
  : Command 22 Off { $0 = 45; }
  : Command 23 On { $0 = 46; }
  : Command 23 Off { $0 = 47; }
  : Command 24 On { $0 = 48; }
  : Command 24 Off { $0 = 49; }
  : Command 25 On { $0 = 50; }
  : Command 25 Off { $0 = 51; }
  : Command 26 On { $0 = 52; }
  : Command 26 Off { $0 = 53; }
  : Command 27 On { $0 = 54; }
  : Command 27 Off { $0 = 55; }
  : Command 28 On { $0 = 56; }
  : Command 28 Off { $0 = 57; }
  : Command 29 On { $0 = 58; }
  : Command 29 Off { $0 = 59; }
  : Command 30 On { $0 = 60; }
  : Command 30 Off { $0 = 61; }
  : Command 31 On { $0 = 62; }
  : Command 31 Off { $0 = 63; }
  ;