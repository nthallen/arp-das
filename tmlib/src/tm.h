/* Copyright 2001 by the President and Fellows of Harvard College
 */
#ifndef TM_H
#define TM_H

#include <sys/types.h> /* this is for pid_t and nid_t and time_t */

#ifndef __GNUC__
  #define __attribute__(x)
#endif

typedef unsigned short mfc_t;

/*
 * Message structures 
 */

/* token info */
typedef unsigned char token_type;

/* Structure used to transmit data around the dbr */
typedef struct {
  token_type n_rows;
  unsigned char data[1];
} __attribute__((packed)) dbr_data_type;

/* Time stamp information */
typedef struct {
  unsigned short mfc_num;
  time_t secs;
} __attribute__((packed)) tstamp_type;

/* dbr client initialization */
/* This will need some tweaking as we learn
   what RCS can and can't do for us
*/
typedef struct {
  char ident[21]; /* 12345678.123,v 12.12 */
} tmid_type;

/* nrowsper/nsecsper = rows/sec */
typedef struct {
  tmid_type    tmid;
  unsigned short nbminf;
  unsigned short nbrow;
  unsigned short nrowmajf;
  unsigned short nsecsper;
  unsigned short nrowsper;
  unsigned short mfc_lsb;
  unsigned short mfc_msb;
  unsigned short synch;
  unsigned short isflag;
} __attribute__((packed)) tm_info_type;
#define ISF_INVERTED 1

typedef struct {
  tm_info_type tm;	    /* data info */
  unsigned short nrowminf;    /* number rows per minor frame */
  unsigned short max_rows;    /* maximum number of rows allowed to be sent in a message */
  unsigned short unused;	    /* tid to send to */
  unsigned short tm_started;  /* flow flag */
  tstamp_type  t_stmp;	    /* current time stamp */
  unsigned char mod_type;   /* module type */
} __attribute__((packed)) dbr_info_type;

/* Function prototypes: */

/* Data Client library functions: */
extern int TM_open_stream(int blocking);
extern int TM_readfd(void);
extern int TM_stream( int nbytes, const char *data );

/*  Data Client application functions: */
extern void TM_data( int datatype, mfc_t mfctr, int row, int nrows, const char *data );
extern void TM_init( void );
extern void TM_row( mfc_t mfctr, int row, const char *data );
extern void TM_tstamp( int tstype, mfc_t mfctr, time_t time );
extern void TM_quit( void );

/* Global variables */
extern dbr_info_type dbr_info;
#define tmi(x) dbr_info.tm.x

#endif
