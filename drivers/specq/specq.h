/* specq.h */
#ifndef SPECQ_H_INCLUDED
#define SPECQ_H_INCLUDED

#define SPECQ_MAX_MSGSIZE 256
void specq_opt_init( int argc, char **argv );

// This is the telemetry data structure
// It will be identified in TM as
// #include "specq.h"
// specq_t SpecQ;
// TM "Receive" SpecQ 0;
// 
// Logical follow ons would be:
// TM typdef unsigned short USHRT;
// TM typedef unsigned char UCHAR;
// TM 1 Hz USHRT SQ_scan;
// TM 1 Hz USHRT SQ_status;
// TM 1 Hz UCHAR SQ_stale;
// group specq (SQ_scan, SQ_status, SQ_stale) {
//    SQ_scan = SpecQ.scannum;
//    SQ_status = SpecQ.status;
//    SQ_stale = SpecQ_obj.stale();
// }
typedef struct {
  unsigned short scannum;
  unsigned short status;
} specq_t;

#define MAX_PROTOCOL_LINE 256

#define SPECQ_EXIT "EX"
#define SPECQ_RESET "RE"
#define SPECQ_CHECK "CK"
#define SPECQ_SCAN  "SC"
#define SPECQ_RESET_SCAN "RN"
#define SPECQ_RESET_STATUS "RS"

#define SPECQ_N_EXIT 2 /* 'EX' */
#define SPECQ_N_RESET 3 /* 'RE' */
#define SPECQ_N_CHECK 4 /* 'CK' */
#define SPECQ_N_SCAN  5 /* 'SC' */
#define SPECQ_N_RESET_SCAN 6 /* 'RN' */
#define SPECQ_N_RESET_STATUS 7 /* 'RS' */

#endif
