/* This include file consists of constants and structures that are shared
 * by all programs involved in DBR data information
 * Written by David Stahl
 * Modified May 21, 1991 by NTA
 * Modified May 22, 1991 by Eil
 * Modified May 23, 1991 by NTA extensively.
 * Modified June 20, 1991, took out R_OK
 * Modified Sep 26, 1991, by Eil, changed from ring to buffered ring. (dbr).
 * Modified by Eil 4/22/92, port to QNX 4.
 $Log$
 Revision 1.2  2000/01/23 14:58:19  nort
 32-bit changes

 Revision 1.1  2000/01/23 14:56:55  nort
 Initial revision

 * Revision 1.6  1992/06/09  20:40:12  eil
 * dbr_info.seq_num
 *
 * Revision 1.4  1992/05/22  16:21:58  eil
 * merge eil and nort
 *
 * Revision 1.3  1992/05/21  18:52:56  nort
 * Split tm_info nrowsec into nrowsper and nsecsper.
 * Extended size of ident to 21 (More structure may be useful later).
 *
 * Revision 1.2  1992/05/20  17:37:58  nort
 * Changed prototype for DG_init().
 * Added prototype for DG_dac_in().
 * Added OPT_DG_* definitions.
 *
 */
 
#ifndef DBR_H
#define DBR_H

#ifndef __QNXNTO__
  #include <sys/name.h>
#endif
#include <sys/types.h> /* this is for pid_t and nid_t and time_t */

#ifndef __GNUC__
  #define __attribute__(x)
#endif

/* only one dg and one db allowed per node */
#define DG_NAME "dg"
#define DB_NAME "db"

/* module types */
#define DRC 0				/* data ring client */
#define DBC 1				/* data buffer client */
#define DG 2				/* data generator */
#define DB 3				/* data buffer */
#define DBCP 4              /* non-blocking buffer client Parent */
#define DBCC 5              /* non-blocking buffer client child */

#define MAX_BUF_SIZE 1000 /* arbitrary size for DBRING message buffers with data */

/* Option string definitions: */
#define OPT_DG_INIT "n:j:"
#define OPT_DC_INIT "b:i:"
#define OPT_DG_DAC_IN "f:"

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

/* Reply to drinit */
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
#ifndef __QNXNTO__
  int  DC_init(int, nid_t);
#endif
int  DC_init_options(int, char ** );
int  DC_operate(void);
int  DC_bow_out(void);

/*  Data Client application functions: */
void DC_data(dbr_data_type *dr_data);
void DC_tstamp(tstamp_type *tstamp);
void DC_DASCmd(unsigned char type, unsigned char number);
void DC_other(unsigned char *msg_ptr, pid_t sent_tid);

/* Data Generator library functions: */
int  DG_init(int start_after_inits_num, int delay_in_ms);
int  DG_dac_in(int argc, char **argv);
int  DG_init_options(int argc, char **argv);
int  DG_operate(void);
void DG_s_data(token_type n_rows, unsigned char *data, token_type n_rows1, unsigned char *data1);
void DG_s_tstamp(tstamp_type *tstamp);
void DG_s_dascmd(unsigned char type, unsigned char val);

/* Data Generator application functions: */
int  DG_DASCmd(unsigned char type, unsigned char val);
int  DG_other(unsigned char *msg_ptr, int sent_tid);
int  DG_get_data(token_type n_rows);

/* Data Buffer library functions */
int  DB_init(void);
void DB_s_data(pid_t who, token_type n_rows, unsigned char  *data, token_type n_rows1, unsigned char *data1);
void DB_s_tstamp(pid_t who, tstamp_type *tstamp);
void DB_s_dascmd(pid_t who, unsigned char type, unsigned char val);
void DB_s_init_client(pid_t who, unsigned char replycode);

/* Data Buffer application functions */
int DB_get_data(pid_t who, token_type n_rows);

/* Global variables declared in dbr_info.c */
 extern dbr_info_type dbr_info;
 #define tmi(x) dbr_info.tm.x
 extern int dbr_breaksignal;

 /* data clients */
 extern token_type DC_data_rows;

#if defined __386__
#  pragma library (dbr3r)
#elif defined __SMALL__
#  pragma library (dbrs)
#elif defined __COMPACT__
#  pragma library (dbrc)
#elif defined __MEDIUM__
#  pragma library (dbrm)
#elif defined __LARGE__
#  pragma library (dbrl)
# endif

#endif
