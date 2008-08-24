/* syscon.h defines addresses and other useful thing for the
   Lightweight System Controller Board.
   Written January 22, 1990
*/
 
#define SC_SB_RESET 0x310
#define SC_SB_PORTA 0x308
#define SC_SB_PORTB 0x30A
#define SC_SB_PORTC 0x30C
#define SC_SB_CONTROL 0x30E
#define SC_SB_CONFIG 0xC1C0
#define SC_CMDENBL 0x318
#define SC_NVADDR 0x31A
#define SC_NVRAM 0x31D
#define SC_TICK 0x319
#define SC_DISARM 0x311
#define SC_INPUT 0x31C
#define SC_NMIENBL 0x31C
#define SC_LOWPOWER (inp(SC_SB_PORTC+1)&4)
#define SC_CMDENBL_BCKPLN (inp(SC_SB_PORTC+1)&2)

/* Non-volatile RAM status values: */
#define RAM_READY 0
#define RAM_INIT 1
#define RAM_RUNNING 2
#define RAM_PFAIL_DET 3
#define RAM_PFAIL_OK 4
#define RAM_FLT_OVER 5
#define RAM_DUMPED 6
#define RAM_PWR_CYC_REQ 7
#define RAM_FAILURE 8
#define RAM_EARLY 9
#define RAM_FLT_OVER2 10
#define RAM_INACTIVE 0xF6
#define RAM_SYSRESET 0xF7
#define RAM_PFDIAG   0xF8
#define RAM_PATTERNS 0xF9

/* card addresses: */
#define H2O_BEG 0xD00
#define H2O_END 0xD0F
#define AtoD0_BEG 0xC00
#define AtoD0_END 0xC1F
#define AtoD1_BEG 0xC20
#define AtoD1_END 0xC3F
#define AtoD2_BEG 0xC40
#define AtoD2_END 0xC5F
#define AtoD3_BEG 0xC60
#define AtoD3_END 0xC7F
#define AtoD4_BEG 0xC80
#define AtoD4_END 0xC9F
#define AtoD5_BEG 0xCA0
#define AtoD5_END 0xCBF
#define AtoD6_BEG 0xCC0
#define AtoD6_END 0xCDF
#define AtoD7_BEG 0xCE0
#define AtoD7_END 0xCFF

/* PC104 stuff */
#define DtoA0_104_BEG 0xC60
#define DtoA0_104_END 0xC70

#define DtoA1_104_BEG 0xCE0
#define DtoA1_104_END 0xCF0

#define DtoA2_104_BEG 0xD60
#define DtoA2_104_END 0xD70

#define DtoA3_104_BEG 0xDE0
#define DtoA3_104_END 0xDF0

#define DtoA4_104_BEG 0xE60
#define DtoA4_104_END 0xE70

#define DtoA5_104_BEG 0xEE0
#define DtoA5_104_END 0xEF0

#define DtoA6_104_BEG 0xF60
#define DtoA6_104_END 0xF70

#define DtoA7_104_BEG 0xFE0
#define DtoA7_104_END 0xFF0

#define AtoD0_104_BEG 0xC10
#define AtoD0_104_END 0xC5E

#define AtoD1_104_BEG 0xC90
#define AtoD1_104_END 0xCDE

#define AtoD2_104_BEG 0xD10
#define AtoD2_104_END 0xD5E

#define AtoD3_104_BEG 0xD90
#define AtoD3_104_END 0xDDE

#define AtoD4_104_BEG 0xE10
#define AtoD4_104_END 0xE5E

#define AtoD5_104_BEG 0xE90
#define AtoD5_104_END 0xEDE

#define AtoD6_104_BEG 0xF10
#define AtoD6_104_END 0xF5E

#define AtoD7_104_BEG 0xF90
#define AtoD7_104_END 0xFDE


