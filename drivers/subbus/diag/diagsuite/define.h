#define ENABLE '+'
#define DISABLE '-'
#define QUALIFIER '/'
#define EQUAL '='
#define USAGE '?'
#define DOT '.'
#define NULCHR '\0'
#define NULSTR "\0"
#define BLANKSTR " "
#define BLANK ' '
#define COLON ':'
#define MAXCMDS 25
#define HELP_FILE "syscon.hlp"
#define M 77
#define m 109
#define A 65
#define a 97
#define B 66
#define b 98
#define S 83
#define s 115
#define D 68
#define d 100
#define R 82
#define r 114
#define W 87
#define w 119
#define L 76
#define l 108
#define PLUS 43
#define MINUS 45
#ifdef __QNX__
#define CR K_ENTER
#define ESCAPE K_ESC
#else
#define CR 10
#define ESCAPE 27
#endif
#define CTRLL 12
#define CTRLR 18
#define CTRLW 23
#define CTRLC 3
#define CTRLRSTR "^R"
#define CTRLWSTR "^W"
#define CTRLLSTR "^L"
#define CTRLCSTR "^C"
#define ESCSTR "ESC|"
#define CRSTR "CR|"
#define KEY_UPSTR "\30"
#define KEY_DOWNSTR "\31"
#define KEY_LSTR "\33"
#define KEY_RSTR "\32"

#define RSTAT "READ"
#define WSTAT "WRITE"
#define CUSTAT "CYCLE UP AND WRITE"
#define CDSTAT "CYCLE DOWN AND WRITE"
#define CRSTAT "CONTINUOUS READ"
#define CWSTAT "CONTINUOUS WRITE"
#define CCSTAT "CONTINUOUS CYCLE"

#define AISTAT "INC ADDRESS"
#define DISTAT "INC DATA"
#define ACUSTAT "ADDRESS CYCLE UP"
#define ACDSTAT "ADDRESS CYCLE DOWN"
#define ADSTAT "DEC ADDRESS"
#define DDSTAT "DEC DATA"
#define STEPSTAT "ENTER HEX STEP"
#define MAXLIMSTAT "ENTER MAX HEX DATA"
#define MINLIMSTAT "ENTER MIN HEX DATA"
#define ADDRSTAT "ENTER HEX ADDRESS"
#define DATASTAT "ENTER HEX DATA"
#define NOP "No Operation"

/* the lengths of the following are fixed */
#define BSTAT "BYTE/"
#define WDSTAT "WORD/"
#define DECSTAT "DEC/"
#define HEXSTAT "HEX/"
#define BINSTAT "BIN/"

/* radix indicators */
#define DEC 0
#define HEX 1
#define BIN 2

/* resolution */
#define BYTERES 0
#define WORDRES 1

#define MAXLINES 18
#define MAXCOLS 70
#define BYTESIZE 8
#define WORDSIZE 16

/* field width */
#define FIELD_ADDR 4 
#define FIELD_DATA (2*BYTESIZE+2)
#define COL_WIDTH (FIELD_ADDR+FIELD_DATA+1)

/* default number of max failures or warnings allowed to be logged */
#define DEFAULT_LIMIT 100

