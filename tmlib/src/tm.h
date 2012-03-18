/* Copyright 2012 by the President and Fellows of Harvard College */

/*! \file tm.h
 * \brief Defines basic telemetry formats
 */

#ifndef TM_H
#define TM_H

#include <sys/types.h> /* this is for pid_t and nid_t and time_t */
#include <fcntl.h> /* for arguments to tm_open_name() */

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

/* Recognized tm_type codes: */
#define TMTYPE_INIT 0x0100
#define TMTYPE_TSTAMP 0x0200
#define TMTYPE_DATA_T1 0x0301
#define TMTYPE_DATA_T2 0x0302
#define TMTYPE_DATA_T3 0x0303
#define TMTYPE_DATA_T4 0x0304
#define TM_HDR_SIZE_T1 6
#define TM_HDR_SIZE_T2 10
#define TM_HDR_SIZE_T3 8
 

/**
 * The time stamp establishes the relationship between
 * minor frame counter and real time. The precision is
 * intended to be at least the minor frame rate.
 * Accuracy depends on many other factors.
 */
typedef struct {
  mfc_t mfc_num;
  time_t secs;
} __attribute__((packed)) tstamp_t;

typedef struct {
  unsigned char version[16]; /* 1.0 etc. contents of VERSION */
  unsigned char md5[16];     /* MD5 digest of core TM frame definitions */
} tmid_t;

/**
 * tm_dac_t defines the core parameters of a telemetry frame
 * including the MD5 digest of the .pcm frame summary, which
 * includes signal names and their placement in the frame.
 * This information in stored in tm.dac, which is stored
 * along with the data. If the tm.dac data does not match,
 * then the format is probably incompatible.
 *
 * note: nrowsper/nsecsper = rows/sec
 */
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

/**
 * tm_info_t extends the tm_dac_t information with some
 * calculated values for easy reference and the current
 * timestamp.
 */
typedef struct {
  tm_dac_t tm;      /* data info */
  unsigned short nrowminf;    /* number rows per minor frame */
  unsigned short max_rows;    /* maximum number of rows allowed to be sent in a message */
  tstamp_t t_stmp;          /* current time stamp */
} tm_info_t;

/**
   tm_data_t1_t applies when tmtype is TMTYPE_DATA_T1
   This structure type can be used when entire minor frames are
   being transmitted. This is true iff nrows is a multiple
   of nrowminf and the first row transmitted is the first row
   (row 0) of the minor frame. MFCtr and Synch can be extracted
   from the data that follows. data consists of n_rows * nbrow
   bytes.
   
   TMbfr will only output TMTYPE_DATA_T1 records when
   nrowminf == 1.
   
   TM data begins at offset 6 of the complete message.
 */
typedef struct {
  tm_hdrw_t n_rows;
  unsigned char data[2];
} __attribute__((packed)) tm_data_t1_t;

/**
   tm_data_t2_t applies when tmtype is TMTYPE_DATA_T2
   This structure type can be used to transmit rows even
   when the whole minor frame is not present since the
   mfctr and rownum of the first row are included in
   the header. Subsequent rows are guaranteed to be
   consecutive. data consists of n_rows * nbrow bytes.
   
   For practical implementation reasons, TMTYPE_DATA_T2
   will be legal only when nrowminf > 1.
   
   TM data begins at offset 10 of the complete message.
 */
typedef struct {
  tm_hdrw_t n_rows;
  mfc_t mfctr;
  mfc_t rownum;
  unsigned char data[2];
} __attribute__((packed)) tm_data_t2_t;

/**
   tm_data_t3_t applies when tmtype is TMTYPE_DATA_T3
   This structure type can be used only in the case where
   nrowminf=1, mfc_lsb=0 and mfc_msb=1. data is compressed
   by stripping off the leading mfctr and trailing synch
   from each minor frame. Hence data consists of
   n_rows * (nbrow-4) bytes. All rows are guaranteed to
   be sequential (since there is no way to determine
   their sequence without the mfctr).
   
   TM data begins at offset 8 of the complete message.
 */
typedef struct {
  tm_hdrw_t n_rows;
  mfc_t mfctr;
  unsigned char data[2];
} __attribute__((packed)) tm_data_t3_t;

/**
   tm_data_t4_t applies when tmtype is TMTYPE_DATA_T4
   This structure type can be used under the same conditions
   as tm_data_t3_t. The difference is the inclusion of a
   cksum dword which can be used to verify the data.
   The algorithm for calculating the cksum has yet to be
   defined.
   
   TM data begins at offset 12 of the complete message buffer.
   
   For the time being, we will reserve this type for
   actual disk storage. TMbfr may support it on input, but
   will disregard the cksum value. lgr/rdr will be tasked
   with calculating and checking the values respectively.
*/
typedef struct {
  tm_hdrw_t n_rows;
  mfc_t mfctr;
  tmcks_t cksum;
  unsigned char data[2];
} __attribute__((packed)) tm_data_t4_t;

/**
 * tm_msg_t is the logical format of TM messages to and
 * from TMbfr.
 */
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

/**
   tm_hdrs_t is a combination of all the header types,
   defining the minimum size struct we need to read
   in to understand the rest of the message. The message
   layout is best understood in the tm_msg_t structure,
   but in actual practice, I will use tm_hdrs_t, then
   access the data directly.
 */
typedef union tm_hdrs {
  char raw[1];
  struct {
    tm_hdr_t hdr;
    union {
      tstamp_t ts;
      struct {
        tm_hdrw_t n_rows;
        tm_hdrw_t mfctr;
        tm_hdrw_t rownum;
      } dhdr;
    } u;
  } s;
} __attribute__((packed)) tm_hdrs_t;

typedef union {
  tm_msg_t msg;
  char raw[TMBUFSIZE];
} tm_packet_t;

/* Global variables */
extern tm_info_t tm_info;
#define tmi(x) tm_info.tm.x
#define tm_mfctr(mf) (mf[tmi(mfc_lsb)] + (mf[tmi(mfc_msb)]<<8))
extern int TM_fd;
extern char *TM_buf;

/* Function prototypes: */
#ifdef __cplusplus
  extern "C" {
#endif

extern const char *tm_dev_name(const char *base);
extern int tm_open_name(const char *name, const char *node, int flags);

/* Command Interpreter Client (CIC) and Server (CIS) Utilities
   Message-level definition is in cmdalgo.h
 */
void cic_options(int argc, char **argv, const char *def_prefix);
int cic_init(void);
void cic_reset(void);
extern char ci_version[];
void cic_transmit(char *buf, int n_chars, int transmit);
int ci_sendcmd(const char *cmdtext, int mode);
int ci_sendfcmd(int mode, const char *fmt, ...);
void ci_settime( long int time );
extern int cic_cmd_quit_fd;
const char *ci_time_str( void );
#define CMD_MAX_COMMAND_IN 300  // Maximum command message input length
void ci_server(void); /* in nortlib/cis.c */
void cis_initialize(void); /* in cmdgen.skel or .cmd */
void cis_terminate(void);  /* in cmdgen.skel of .cmd */
int ci_cmdee_init( const char *name );
void ci_report_version(void);

/* tmcalgo (tma) support routines */
void tma_new_state(unsigned int partition, const char *name);
void tma_new_time(unsigned int partition, long int t1, const char *next_cmd);
void tma_hold(int hold);

/* tmdac.c */
void load_tmdac( const char *path );

#ifdef __cplusplus
  };
#endif

#endif
