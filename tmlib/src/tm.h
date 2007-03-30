/* Copyright 2001 by the President and Fellows of Harvard College
 */
#ifndef TM_H
#define TM_H

#include <sys/types.h> /* this is for pid_t and nid_t and time_t */

#ifndef __GNUC__
  #define __attribute__(x)
#endif

/*
 * Message structures 
 * I am assuming a short has 16 bits and a long has 32 bits
 * should probably add:
 *   assert( sizeof(tm_hdrw_t) == 2 );
 *   assert( sizeof(tmcks_t) == 4 );
 * to the initialization somewhere.
 */
typedef unsigned short mfc_t;
typedef unsigned short tm_hdrw_t;
typedef unsigned long tmcks_t;
typedef struct {
  tm_hdrw_t tm_id; /* 'TM' TMHDR_WORD */
  tm_hdrw_t tm_type;
} tm_hdr_t;
#define TMHDR_WORD 0x4D54
#define TMBUFSIZE 16384
#define TM_DEV_BASE "/dev/huarp"
#define TM_DEV_SUFFIX "TM"

/* Recognized tm_type codes: */
#define TMTYPE_INIT 0x0100
#define TMTYPE_TSTAMP 0x0200
#define TMTYPE_DATA_T1 0x0301
#define TMTYPE_DATA_T2 0x0302
#define TMTYPE_DATA_T3 0x0303
#define TMTYPE_DATA_T4 0x0304

/* These are not TMTYPEs. DASCMDs will be handled
   on a separate channel
 */
#define TMTYPE_DASCMD  0x0400
#define TMTYPE_DASCMD_START   0x0401
#define TMTYPE_DASCMD_QUIT    0x0402
#define TMTYPE_DASCMD_LOGSUS  0x0410
#define TMTYPE_DASCMD_LOGRES  0x0411
#define TMTYPE_DASCMD_STOP    0x0420
#define TMTYPE_DASCMD_PLAY    0x0421
#define TMTYPE_DASCMD_FASTER  0x0422
#define TMTYPE_DASCMD_SLOWER  0x0423
#define TMTYPE_DASCMD_FF      0x0424
#define TMTYPE_DASCMD_FFMFC   0x0425
#define TMTYPE_DASCMD_FFTIME  0x0426
#define TMTYPE_DASCMD_STEP    0x0427
#define TMTYPE_DASCMD_RESTART 0x0428
#define TMTYPE_DASCMD_RWMFC   0x0429
#define TMTYPE_DASCMD_RWTIME  0x042A

/* Time stamp information */
typedef struct {
  mfc_t mfc_num;
  time_t secs;
} __attribute__((packed)) tstamp_t;

typedef struct {
  char version[16]; /* 1.0 etc. contents of VERSION */
  char md5[16];     /* MD5 digest of core TM frame definitions */
} tmid_t;

/* nrowsper/nsecsper = rows/sec */
typedef struct {
  tmid_t    tmid;
  tm_hdrw_t nbminf;
  tm_hdrw_t nbrow;
  tm_hdrw_t nrowmajf;
  tm_hdrw_t nsecsper;
  tm_hdrw_t nrowsper;
  tm_hdrw_t mfc_lsb;
  tm_hdrw_t mfc_msb;
  tm_hdrw_t synch;
  tm_hdrw_t flags;
} __attribute__((packed)) tm_dac_t;
#define TMF_INVERTED 1

typedef struct {
  tm_dac_t tm;	    /* data info */
  unsigned short nrowminf;    /* number rows per minor frame */
  unsigned short max_rows;    /* maximum number of rows allowed to be sent in a message */
  tstamp_t t_stmp;	    /* current time stamp */
} tm_info_t;

/* tm_data_t1_t applies when tmtype is TMTYPE_DATA_T1
   This structure type can be used when entire minor frames are
   being. This is true iff nrowminf is a multiple of n_rows and
   the first row transmitted is the first row (row 0) of the
   minor frame. MFCtr and Synch can be extracted from the data
   that follows. data consists of n_rows * nbrow bytes. */
typedef struct {
  tm_hdrw_t n_rows;
  unsigned char data[2];
} __attribute__((packed)) tm_data_t1_t;

/* tm_data_t2_t applies when tmtype is TMTYPE_DATA_T2
   This structure type can be used to transmit rows even
   when the whole minor frame is not present since the
   mfctr and rownum of the first row are included in
   the header. Subsequent rows are guaranteed to be
   consecutive. data consists of n_rows * nbrow bytes. */
typedef struct {
  tm_hdrw_t n_rows;
  mfc_t mfctr;
  mfc_t rownum;
  unsigned char data[2];
} __attribute__((packed)) tm_data_t2_t;

/* tm_data_t3_t applies when tmtype is TMTYPE_DATA_T3
   This structure type can be used only in the case where
   nrowminf=1, mfc_lsb=0 and mfc_msb=1. data is compressed
   by stripping off the leading mfctr and trailing synch
   from each minor frame. Hence data consists of
   n_rows * (nbrow-4) bytes. All rows are guaranteed to
   be sequential (since there is no way to determine
   their sequence without the mfctr). */
typedef struct {
  tm_hdrw_t n_rows;
  mfc_t mfctr;
  unsigned char data[2];
} __attribute__((packed)) tm_data_t3_t;

/* tm_data_t4_t applies when tmtype is TMTYPE_DATA_T4
   This structure type can be used under the same conditions
   as tm_data_t3_t. The difference is the inclusion of a
   cksum dword which can be used to verify the data.
   The algorithm for calculating the cksum has yet to be
   defined. */
typedef struct {
  tm_hdrw_t n_rows;
  mfc_t mfctr;
  tmcks_t cksum;
  unsigned char data[2];
} __attribute__((packed)) tm_data_t4_t;

typedef struct tm_msg {
  tm_hdr_t hdr;
  union {
	tstamp_t ts;
	tm_info_t init;
	tm_data_t1_t data1;
	tm_data_t2_t data2;
	tm_data_t3_t data3;
	tm_data_t4_t data4;
  } body;
} __attribute__((packed)) tm_msg_t;

typedef union {
  tm_msg_t msg;
  char raw[TMBUFSIZE];
} tm_packet_t;

/* Function prototypes: */

/* Data Client library functions: */
extern int TM_open_stream( int write, int nonblocking );
extern int TM_read_stream( void );
extern int TM_readfd(void);
extern int TM_stream( int nbytes, const char *data );

/*  Data Client application functions: */
extern void TM_data( tm_msg_t *msg, int n_bytes );
extern void TM_init( void );
extern void TM_row( mfc_t mfctr, int row, const unsigned char *data );
extern void TM_tstamp( int tstype, mfc_t mfctr, time_t time );
extern void TM_quit( void );

/* Global variables */
extern tm_info_t tm_info;
#define tmi(x) tm_info.tm.x
extern int TM_fd;
extern char *TM_buf;

#endif
