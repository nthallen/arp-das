; clo.sol
; test version oct 15 1993

solenoid Air0 10 11 4		; Air Sol 0
solenoid Air1 12 13 2		; Air Sol 1
solenoid NO 22 23 6		; NO Sol
solenoid NOFlC 14 15 0		; NO Flow Closed
solenoid GateC 18 19 2		; Gate Valve Closed
solenoid LabFl1 24 25 0		; Lab Flow 1 Closed
solenoid LabFl2 28 29 4		; Lab Flow 2 Closed

dtoa NOFlw0 0xE42 {K:2047 L:1500 M:1200 N:900 P:600 Q:300 T:2 A:1 Z:0}
dtoa NOFlw1 0xE42 {G:2000 H:1800 I:1600 U:1400 V:1200 W:1000 X:0}
dtoa NOFlw2 0xE42 {B:800 C:600 D:400 E:200 F:0}
dtoa LFl1Sp 0xE40 {}
dtoa LFl2Sp 0xE44 {}
dtoa LFl3Sp 0xE46 {}
dtoa LFl4Sp 0xE50 {}
dtoa Status 0x00  {A:0 B:1 C:2 D:3 E:4 F:5 G:6 H:7 I:8 J:9}

Command_Set = 'A';

resolution = 1/1

mode 0 {
  Status: A
  NO:     _
  NOFlC:    O
  NOFlw0: Z
  AIR1:    _
}

mode 1 {
  Status:  BBBBB:BBBB:BBBBB:BBBBBB:BBBB:BBBB:
  AIR1:    _____:____:_____:______:OOOO:OOOO:^
}

; flight
mode 3 {
  Status:  CCCCC:CCCC:CCCC:CCCC:CCCCCC:CCCCC:CCCC:CCCC:CCCC:CCCCCC:
  AIR1:    OOOOO:OOOO:OOOO:OOOO:OOOOOO:OOOOO:OOOO:OOOO:OOOO:OOOOOO:
  NO:     _____:____:_OOO:OOOO:OOOOOO:_____:____:_OOO:OOOO:OOOOOO:
  NOFlC:    OOOOO:OOOO:____:____:______:OOOOO:OOOO:____:____:______:
  NOFlw0: AZZZZ:ZZZZ:KKKK:LLMM:NNPPQQ:AZZZZ:ZZZZ:KKKK:LLMM:NNPPQQ:

  Status:  CCCCC:CCCC:CCCC:CCCC:CCCCCC:CCCCC:
  AIR1:    OOOOO:OOOO:OOOO:OOOO:OOOOOO:OOOOO:
  NO:     _____:____:_OOO:OOOO:OOOOOO:_____:
  NOFlC:    OOOOO:OOOO:____:____:______:OOOOO:
  NOFlw0: AZZZZ:ZZZZ:KKKK:LLMM:NNPPQQ:TZZZZ:^
}


mode 4 {
  Status: DDDD
  AIR1:    ____
  NO:     ____
  NOFlC:    OOOO
  NOFlw0: ZZZZ
  Select 0
}


; absorbtion
mode 6 {
  Status:   FFFFFF:FFFFF:FFFFFF:FFFFF:FFFFFF:FFFFF:
  NOFlC:    OOOOOO:OOOOO:OOOOOO:OOOOO:OOOOOO:OOOOO:
  NOFlw0: AZZZZZ:KKKKK:AZZZZZ:MMMMM:AZZZZZ:PPPPP:^

  Status:   FFFFFF:FFFFF:FFFFFF:FFFFF:FFFFFF:FFFFF:
  NOFlC:    OOOOOO:OOOOO:OOOOOO:OOOOO:OOOOOO:OOOOO:
  NOFlw0: AZZZZZ:KKKKK:AZZZZZ:MMMMM:AZZZZZ:PPPPP:^
}

;calibration
mode 7 {
  Status:   GGGG:GGGG:GGGG:GGGG:GGGG:GGGG:GGGG:GGGG:
  NOFlC:    ____:____:____:____:____:____:____:____:
  NOFlw1: XXXX:XXXX:GGGG:GGGG:HHHH:HHHH:IIII:IIII:^

  Status:   GGGG:GGGG:GGGG:GGGG:GGGG:GGGG:
  NOFlC:    ____:____:____:____:____:____:
  NOFlw1: UUUU:UUUU:VVVV:VVVV:WWWW:WWWW:^
Select 8
}

mode 8 {
  Status:   HHHH:HHHH:HHHH:HHHH:HHHH:HHHH:HHHH:HHHH:HHHH:HHHH:
  NOFlC:    ____:____:____:____:____:____:____:____:____:____:
  NOFlw2: BBBB:BBBB:CCCC:CCCC:DDDD:DDDD:EEEE:EEEE:FFFF:FFFF:^
Select 7
}
