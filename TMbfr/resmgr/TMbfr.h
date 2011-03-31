/* Copyright 2007 by the President and Fellows of Harvard College */
#ifndef TMBFR_H_INCLUDED
#define TMBFR_H_INCLUDED
#include <pthread.h>
#include "tm.h"

struct tm_ocb;
struct tm_attr;
#define THREAD_POOL_PARAM_T dispatch_context_t
#define IOFUNC_OCB_T struct tm_ocb
#define IOFUNC_ATTR_T struct tm_attr
#include <sys/iofunc.h>
#include <sys/dispatch.h>

/* Semantics of Data_Queue
   Data_Queue.first, .last are indices into row and range from
     [0..total_Qrows)
   .first is where the next row will be read from
   .last is where the next row will be written to
   first==last is empty unless full is asserted
*/
typedef struct dataqueue {
  char *raw;
  char **row;
  tm_hdrw_t output_tm_type;
  int pbuf_size; // nbQrow+nbDataHdr (or more)
  int total_size;
  int total_Qrows;
  int nbQrow; // may differ from nbrow if stripping MFCtr & Synch
  int nbDataHdr;
  int first;
  int last;
  int full;
  int nonblocking;
} data_queue_t;

extern data_queue_t Data_Queue; // There can be only one

typedef struct tsqueue {
  int ref_count;
  tstamp_t TS;
} TS_queue_t;

/* Semantics of the dq_descriptor
   next points to a later descriptor. A separate descriptor is
     required when a new TS arrives or a row is skipped.
   ref_count indicates how many OCBs point to this dqd
   starting_Qrow is the index into Data_Queue.row for the first data
     row of this dqd that is still present in the Data_Queue, or the location
     of the next record if no rows are present.
   n_Qrows is the number of Qrows of this dqd still present in the
     Data_Queue, hence must be <= Data_Queue.total_Qrows.
   Qrows_expired indicates the number of Qrows belonging to this dqd
     that have been expired out of the Data_Queue
    min_reader is the number of rows in this dqd that the slowest
      reader has processed (including expired rows)
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
  int ref_count;
  int starting_Qrow;
  int n_Qrows;
  int Qrows_expired;
  int min_reader;
  TS_queue_t *TSq;
  mfc_t MFCtr;
  int Row_num;
} dq_descriptor_t;

typedef struct {
  dq_descriptor_t *first;
  dq_descriptor_t *last;
} DQD_Queue_t;

extern DQD_Queue_t DQD_Queue;

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
   header+1row in the ocb->part.buf and increment ocb->data.n_Qrows
   (since we're done with the row held in the dq now that we've copied
   it.)
   
   ocb->data.n_Qrows is the number of rows from the start of dqd,
   including expired rows.
   
   On write:
     nbrow_rec and nbhdr_rec are set when the first data record
     arrives. It is an error to change source data formats mid-stream.
     
     The big challenge on writing is keeping track of where we are
     with respect to the received message, the current TM record and
     the destination buffer. The destination buffer is always less
     than or equal to the TM record size, but the record size may be
     smaller or larger than the message size.
     
     off_queue keeps track of the number of bytes that have been
     copied into the Data_Queue. When the current transfer is
     completed (part.nbdata == 0), the Data_Queue and the affected
     dqds are updated to incorporate the new data.
     
     ocb->part.nbdata records how many bytes are still expected in this sub-transfer
     For writes, this means how many bytes we must receive before we can take
     action. For reads, it means how many bytes are currently ready for transfer
     at ocb->part.nbdata.
     
*/
typedef struct tm_ocb {
  iofunc_ocb_t hdr;
  int state;
  struct tm_ocb *next_ocb;
  struct part_s {
    tm_hdrs_t hdr;
    char *dptr; // pointer into other buffers
    int nbdata; // How many bytes are still expected in this sub-transfer
  } part;
  struct data_s {
    dq_descriptor_t *dqd; // Which dq_desc we reference
    int n_Qrows; // The number of Qrows in dq we have already processed
  } data;
  union rw_u {
    struct write_s {
      char *buf; // allocated temp buffer
      int rcvid; // Who is writing
      int nbrow_rec; // bytes per row received
      int nbhdr_rec; // bytes in the header of data messages
      int nb_msg; // bytes remaining in this write
      int off_msg; // bytes already read from this write
      int nb_rec; // bytes remaining in this record
      int off_queue; // bytes already read in this queue block
      // int off_rec; // bytes read in this record: redundant
      // int nb_queue; // bytes remaining in this queue block
      // deemed redundant with part.nbdata
    } write;
    struct read_s {
      char *buf; // allocated temp buffer
      int rcvid; // Who requested
      int nbytes; // size of request
      int maxQrows; // max number of Qrows to be returned with this request
      int rows_missing; // cumulative count
      int blocked; // non-zero if we are blocking
      int ionotify; // non-zero if we want to be notified
    } read;
  } rw;
} tm_ocb_t;

#define TM_STATE_HDR 0
#define TM_STATE_INFO 1
#define TM_STATE_DATA 2

/* Just Identify which node */
typedef struct tm_attr {
  iofunc_attr_t attr;
  int node_type;
  iofunc_notify_t notify[3];  /* notification list used by iofunc_notify*() */
} tm_attr_t;
#define TM_DG 1
#define TM_DCf 2
#define TM_DCo 3

#endif
