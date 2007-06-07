/* Copyright 2007 by the President and Fellows of Harvard College */
#ifndef TMBFR_H_INCLUDED
#define TMBFR_H_INCLUDED
#include <pthread.h>
#include "tm.h"

#define THREAD_POOL_PARAM_T dispatch_context_t
#define IOFUNC_OCB_T struct tm_ocb
#define IOFUNC_ATTR_T struct tm_attr
#include <sys/iofunc.h>
#include <sys/dispatch.h>

typedef struct dataqueue {
  unsigned char *raw;
  unsigned char **row;
  tm_hdrw_t output_tm_type;
  int pbuf_size;
  int total_size;
  int total_Qrows;
  int nbQrow; // may differ from nbrow if stripping MFCtr & Synch
  int first;
  int last;
} data_queue_t;

extern data_queue_t Data_Queue; // There can be only one

typedef struct tsqueue {
  struct tsqueue *next;
  int ref_count;
  tstamp_t TS;
} TS_queue_t;

extern TS_queue_t *TS_Queue;

/* Semantics of the dq_descriptor
   next points to a later descriptor. A separate descriptor is
     required when a new TS arrives or a row is skipped.
   ref_count indicates how many OCBs point to this dqd
   starting_Qrow is the index into Data_Queue.row for the first data
     row of this dqd that is still present in the Data_Queue.
   n_Qrows is the number of Qrows of this dqd still present in the
     Data_Queue
   Qrows_expired indicates the number of Qrows belonging to this dqd
     that have been expired out of the Data_Queue
   TSq is the TS record our data is tied to
   MFCtr is the MFCtr for the starting row (possibly expired) of this
     dqd
   Row_num is the row number (0 <= Row_num < nrowminf) for the
     starting row (possibly expired) of this dqd

   n_Qrows + Qrows_expired is the total number of Qrows for this dqd
   
   To get the MFCtr and Row_Num for the current first row:
   XRow_Num = Row_Num + Qrows_expired
   NMinf = XRow_Num/tm->nrowminf
   MFCtr_start = MFCtr + NMinf
   Row_Num_start = XRow_Num % tm->nrowminf
   
   If n_Qrows == 0 and Qrows_expired == 0, MFCtr and Row_num can be
   redefined. After that, progress simply involves updating
   starting_Qrow, n_Qrows and Qrows_expired.
*/
typedef struct dq_descriptor {
  struct dq_descriptor *next;
  int ref_count; // number of OCBs point to this record
  int starting_Qrow; // Qrow number in the Data_Queue
  int n_Qrows;
  int Qrows_expired;
  TS_queue_t *TSq;
  mfc_t MFCtr;
  int Row_num;
} dq_descriptor_t;

extern dq_descriptor_t *DQD_Queue;

/* I have grouped related members into structs here purely
   to help make clear which members are related.
   If you don't like this approach, let me know.
   
   Each OCB needs to point to a dq_descriptor and record its
   position within that descriptor. This may not apply exactly
   to the writer.
   
   Also need a buffer to store partial frames when a request is small.
   It may be reasonable to delay allocation of this buffer, since
   most apps will request complete frames.
   
   If a read request for a partial row arrives, we buffer the entire
   header+1row in the ocb->part.buf and increment ocb->read.n_Qrows
   (since we're done with the row held in the dq now that we've copied
   it.)
   
*/
typedef struct tm_ocb {
  iofunc_ocb_t hdr;
  int state;
  struct tm_ocb *next_ocb;
  struct {
    tm_hdrs_t hdr;
    char *buf; // allocated temp buffer
    char *dptr; // pointer into other buffers
    int nbdata; // How many bytes are still expected
    int off; // How many bytes have already been received
    // off and dptr are now redundant. Eliminate off?
  } part;
  struct {
    dq_descriptor_t *dqd; // Which dq_desc we reference
    int n_Qrows; // The number of Qrows in dq we have already processed
  } data;
  struct {
    int rcvid; // Who requested
    int nbytes; // size of request
    int rows_missing; // TBD
  } read;
} tm_ocb_t;

#define TM_STATE_HDR 0
#define TM_STATE_INFO 1
#define TM_STATE_DATA 2

/* Just Identify which node */
typedef struct tm_attr {
  iofunc_attr_t attr;
  int node_type;
} tm_attr_t;
#define TM_DG 1
#define TM_DCf 2
#define TM_DCo 3

#endif
