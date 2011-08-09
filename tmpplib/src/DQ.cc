/* DataQueue.c */
#include "DQ.h"
#include "nortlib.h"
#include "nl_assert.h"

/**
 * Base class for dq_data_ref and dq_tstamp_ref
 * These make up part of the control structure of data_queue.
 */
dq_ref::dq_ref(dqtype mytype) {
  next_dqr = 0;
  type = mytype;
}

/**
 * Convenience function to append an item to the end of the linked list and return the new item
 */
dq_ref *dq_ref::next(dq_ref *dqr) {
  next_dqr = dqr;
  return dqr;
}

dq_data_ref::dq_data_ref(mfc_t MFCtr, int mfrow, int Qrow_in, int nrows_in )
      : dq_ref(dq_data) {
  MFCtr_start = MFCtr_next = MFCtr;
  row_start = row_next = mfrow;
  Qrow = Qrow_in;
  n_rows = 0;
  append_rows( nrows_in );
}

void dq_data_ref::append_rows( int nrows ) {
  row_next += nrows;
  MFCtr_next += row_next/tm_info.nrowminf;
  row_next = row_next % tm_info.nrowminf;
  n_rows += nrows;
}

dq_tstamp_ref::dq_tstamp_ref( mfc_t MFCtr, time_t time ) : dq_ref(dq_tstamp) {
  TS.mfc_num = MFCtr;
  TS.secs = time;
}

/**
  * data_queue base class constructor.
  * Determines the output_tm_type and allocates the queue storage.
  */
data_queue::data_queue( int n_Qrows, int low_water ) {
  total_Qrows = n_Qrows;
  dq_low_water = low_water;
  if ( low_water > n_Qrows )
    nl_error( 3, "low_water must be <= n_Qrows" );

  raw = 0;
  row = 0;
  first = last = 0;
  full = false;
  last_dqr = first_dqr = 0;
}

/**
 * General DG initialization. Assumes tm_info structure has been defined.
 * Establishes the connection to the TMbfr, specifying the O_NONBLOCK option for collection.
 * Initializes the queue itself.
 * Creates dispatch queue and registers "DG/cmd" device and initializes timer.
 */
void data_queue::init() {
  // Determine the output_tm_type
  nbQrow = tmi(nbrow);
  tm_info.nrowminf = tmi(nbminf)/tmi(nbrow);
  if (tm_info.nrowminf > 2) {
    output_tm_type = TMTYPE_DATA_T2;
    nbDataHdr = 10;
  } else if ( tmi(mfc_lsb)==0 && tmi(mfc_msb)==1 ) {
    output_tm_type = TMTYPE_DATA_T3;
    nbQrow -= 4;
    nbDataHdr = 8;
  } else {
    output_tm_type = TMTYPE_DATA_T1;
    nbDataHdr = 6;
  }
  if (nbQrow <= 0) nl_error(3,"nbQrow <= 0");
  int total_size = nbQrow * total_Qrows;
  raw = new unsigned char[total_size];
  if ( ! raw )
    nl_error( 3, "memory allocation failure: raw" );
  row = new unsigned char*[total_Qrows];
  if ( ! row )
    nl_error( 3, "memory allocation failure: row" );
  int i;
  unsigned char *currow = raw;
  for ( i = 0; i < total_Qrows; i++ ) {
    row[i] = currow;
    currow += nbQrow;
  }
}

void data_queue::lock(const char * by, int line) {
  by = by;
  line = line;
}

void data_queue::unlock() {}

/**
  no longer a blocking function. Returns the largest number of contiguous rows currently free.
  Caller can decide whether that is adequate.
  Assumes the DQ is locked.
  */
int data_queue::allocate_rows(unsigned char **rowp) {
  int na;
  if ( full ) na = 0;
  else if ( first > last ) {
    na = first - last;
  } else na = total_Qrows - last;
  if ( rowp != NULL) *rowp = row[last];
  return na;
}

/**
 *  MFCtr, mfrow are the MFCtr and minor frame row of the first row being committed.
 * Does not signal whoever is reading the queue
 * Assumes DQ is locked and unlocks before exit
 */
void data_queue::commit_rows( mfc_t MFCtr, int mfrow, int nrows ) {
  // we (the writer thread) own the last pointer, so we can read it without a lock,
  // but we must lock before writing
  nl_assert( !full );
  nl_assert( last+nrows <= total_Qrows );
  lock(__FILE__,__LINE__);
  // We need a new dqr if the last one is a dq_tstamp or my MFCtr,mfrow don't match the 'next'
  // elements in the current dqr
  dq_data_ref *dqdr = 0;
  if ( last_dqr && last_dqr->type == dq_data ) {
    dqdr = (dq_data_ref *)last_dqr;
    if ( MFCtr != dqdr->MFCtr_next || mfrow != dqdr->row_next )
      dqdr = 0;
  }
  if ( dqdr == 0 ) {
    dqdr = new dq_data_ref(MFCtr, mfrow, last, nrows); // or retrieve from the free list?
    if ( last_dqr )
      last_dqr = last_dqr->next(dqdr);
    else first_dqr = last_dqr = dqdr;
  } else dqdr->append_rows(nrows);
  last += nrows;
  if ( last == total_Qrows ) last = 0;
  if ( last == first ) full = 1;
  unlock();
}

/**
 * Does not signal whoever is reading the queue
 */
void data_queue::commit_tstamp( mfc_t MFCtr, time_t time ) {
  dq_tstamp_ref *dqt = new dq_tstamp_ref(MFCtr, time);
  lock(__FILE__,__LINE__);
  if ( last_dqr ) last_dqr = last_dqr->next(dqt);
  else first_dqr = last_dqr = dqt;
  unlock();
}
void data_queue::retire_rows( dq_data_ref *dqd, int n_rows ) {
  lock(__FILE__,__LINE__);
  nl_assert( n_rows >= 0 );
  nl_assert( dqd == first_dqr );
  nl_assert( dqd->n_rows >= n_rows);
  nl_assert( dqd->Qrow == first );
  if ( first < last ) {
    first += n_rows;
    if ( first > last )
      nl_error( 4, "Underflow in retire_rows" );
  } else {
    first += n_rows;
    if ( first >= total_Qrows ) {
      first -= total_Qrows;
      if ( first > last )
        nl_error( 4, "Underflow after wrap in retire_rows" );
    }
  }
  if (n_rows > 0) full = false;
  dqd->Qrow = first;
  dqd->n_rows -= n_rows;
  if ( dqd->n_rows == 0 && dqd->next_dqr ) {
    first_dqr = dqd->next_dqr;
    delete( dqd );
  } else {
    dqd->row_start += n_rows;
    dqd->MFCtr_start += dqd->row_start / tm_info.nrowminf;
    dqd->row_start %= tm_info.nrowminf;
  }
  unlock();
}

void data_queue::retire_tstamp( dq_tstamp_ref *dqts ) {
  lock(__FILE__,__LINE__);
  nl_assert( dqts == first_dqr );
  first_dqr = dqts->next_dqr;
  if ( first_dqr == 0 ) last_dqr = first_dqr;
  unlock();
  delete(dqts);
}
