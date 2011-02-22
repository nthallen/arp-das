; laser.sol
; modified 4/17/93 for spade flight series
; guess flows M,N,P,A,Z,T are best for 60 m/s
; and K,L,Q useful for diagnostic in test flights 
; modified 4/2/93 for calib, test flights and absorbtion
; changed lowest two flows on 4/25/93 to be sure to find
; the ho2 peak at low velocities old 600,300 new 550,450
; modified 5/01/93 to keep air on longer after NO shut off.
; modified 5/15/93 back to flows 600 and 300
solenoid A1IN  20 21 0      ; Axis 1 In
solenoid A1OUT 22 23 1      ; Axis 1 Out
solenoid A2IN  24 25 2      ; Axis 2 In
solenoid A2OUT 26 27 3      ; Axis 2 Out

solenoid NO  6 7 4            ; NO Solenoid
solenoid NOC 3 2 5            ; NO Flow Closed
solenoid AIR 8 9 6            ; Air Solenoid loop injector
solenoid C3F6 0 1 7           ; C3F6 Solenoid

dtoa NOFlw0 0xE44 {K:2047 L:1500 M:1200 N:900 P:600 Q:300 T:2 A:1 Z:0}
dtoa NOFlw1 0xE44 {G:2000 H:1800 I:1600 U:1400 V:1200 W:1000 X:0}
dtoa NOFlw2 0xE44 {B:800 C:600 D:400 E:200 F:0}

Proxy Etalon { O:3 _:4 }
Command_Set = 'A'
Resolution = 1/1

mode 0 {
  Etalon: O
  NO:     _
  NOC:    O
  NOFlw0: Z
  AIR:    _
  A1IN:   _
  A1OUT:  _
  A2IN:   _
  A2OUT:  _
}

mode 1 {
  Etalon: _____:OOOO:_____:OOOOOO:____:OOOO:
  C3F6:   O____:____:_____:______:____:____:
  A1IN:   _____:____:OOO__:______:____:____:
  A1OUT:  OOO__:____:_____:______:____:____:
  A2IN:   _____:____:OOO__:______:____:____:
  A2OUT:  OOO__:____:_____:______:____:____:
  AIR:    _____:____:_____:______:OOOO:OOOO:^
}

mode 2 {
  Etalon: ____:OOOO^
}

mode 3 {
  Etalon: _____:OOOO:_____:OOOOOO:____:OOOO:
  C3F6:   O____:____:_____:______:____:____:
  A1IN:   _____:____:OOO__:______:____:____:
  A1OUT:  OOO__:____:_____:______:____:____:
  A2IN:   _____:____:OOO__:______:____:____:
  A2OUT:  OOO__:____:_____:______:____:____:
  AIR:    _____:____:_____:______:OOOO:OOOO:^
}

mode 4 {
  Etalon: OOOO
  A1IN:   OOO_
  A1OUT:  ____
  A2IN:   OOO_
  A2OUT:  ____
  AIR:    ____
  NO:     ____
  NOC:    OOOO
  NOFlw0: ZZZZ
  C3F6:   ____
  Select 0
}

; flight
mode 5 {
  A1IN:   OOO__:____:____:____:______:_____:____:____:____:______:
  A1OUT:  _____:____:____:____:______:_____:____:____:____:______:
  A2IN:   OOO__:____:____:____:______:_____:____:____:____:______:
  A2OUT:  _____:____:____:____:______:_____:____:____:____:______:
  Etalon: ___OO:OOOO:__OO:OOOO:OOOOOO:___OO:OOOO:__OO:OOOO:OOOOOO:
  AIR:    OOOOO:OOOO:OOOO:OOOO:OOOOOO:OOOOO:OOOO:OOOO:OOOO:OOOOOO:
  NO:     _____:____:_OOO:OOOO:OOOOOO:_____:____:_OOO:OOOO:OOOOOO:
  NOC:    OOOOO:OOOO:____:____:______:OOOOO:OOOO:____:____:______:
  NOFlw0: AZZZZ:ZZZZ:KKKK:LLMM:NNPPQQ:AZZZZ:ZZZZ:KKKK:LLMM:NNPPQQ:


  A1IN:   _____:____:____:____:______:_____:
  A1OUT:  _____:____:____:____:______:OOO__:
  A2IN:   _____:____:____:____:______:_____:
  A2OUT:  _____:____:____:____:______:OOO__:
  Etalon: ___OO:OOOO:__OO:OOOO:OOOOOO:___OO:
  AIR:    OOOOO:OOOO:OOOO:OOOO:OOOOOO:OOOOO:
  NO:     _____:____:_OOO:OOOO:OOOOOO:_____:
  NOC:    OOOOO:OOOO:____:____:______:OOOOO:
  NOFlw0: AZZZZ:ZZZZ:KKKK:LLMM:NNPPQQ:TZZZZ:^
}

; absorbtion
mode 6 {
  A1IN:   ______:_____:______:_____:______:_____:
  A1OUT:  OOO___:_____:______:_____:______:_____:
  A2IN:   ______:_____:______:_____:______:_____:
  A2OUT:  OOO___:_____:______:_____:______:_____:
  Etalon: OOOOOO:OOOOO:OOOOOO:OOOOO:OOOOOO:OOOOO:
  NOC:    OOOOOO:OOOOO:OOOOOO:OOOOO:OOOOOO:OOOOO:
  NOFlw0: AZZZZZ:KKKKK:AZZZZZ:MMMMM:AZZZZZ:PPPPP:^


  A1IN:   ______:_____:______:_____:______:_____:
  A1OUT:  OOO___:_____:______:_____:______:_____:
  A2IN:   ______:_____:______:_____:______:_____:
  A2OUT:  OOO___:_____:______:_____:______:_____:
  Etalon: OOOOOO:OOOOO:OOOOOO:OOOOO:OOOOOO:OOOOO:
  NOC:    OOOOOO:OOOOO:OOOOOO:OOOOO:OOOOOO:OOOOO:
  NOFlw0: AZZZZZ:KKKKK:AZZZZZ:MMMMM:AZZZZZ:PPPPP:^
}

;calibration
mode 7 {
  Etalon: __OO:OOOO:__OO:OOOO:__OO:OOOO:__OO:OOOO:
  NOC:    ____:____:____:____:____:____:____:____:
  NOFlw1: XXXX:XXXX:GGGG:GGGG:HHHH:HHHH:IIII:IIII:^

  Etalon: __OO:OOOO:__OO:OOOO:__OO:OOOO:
  NOC:    ____:____:____:____:____:____:
  NOFlw1: UUUU:UUUU:VVVV:VVVV:WWWW:WWWW:^
Select 8
}

mode 8 {
  Etalon: __OO:OOOO:__OO:OOOO:__OO:OOOO:__OO:OOOO:__OO:OOOO:
  NOC:    ____:____:____:____:____:____:____:____:____:____:
  NOFlw2: BBBB:BBBB:CCCC:CCCC:DDDD:DDDD:EEEE:EEEE:FFFF:FFFF:^
Select 7
}