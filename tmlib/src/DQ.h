#ifndef DQ_H_INCLUDED
#define DQ_H_INCLUDED
#include "tm.h"

// I prefer not ot allocate and free these structures routinely, but I'll start that way.
// It makes sense to keep a free list for the basic types: tstamp_q, dq_tstamp_ref and dq_data_ref
// Actually, I guess that can be optimized via definition of new and delete operators.

// Define a hierarchy here. A dq_descriptor can either hold
// a timestamp or reference data rows in the DQ.
// This works within DG because we don't have readers starting
// and stopping. We have exactly one reader that will go
// through all the data.
enum dqtype { dq_tstamp, dq_data  };

class dq_ref {
  public:
    dq_ref(dqtype mytype);
    dq_ref *next(dq_ref *dqr);
    dq_ref *next_dqr;
    dqtype type;
};

class dq_tstamp_ref : public dq_ref {
  public:
    dq_tstamp_ref( mfc_t MFCtr, time_t time );
    tstamp_t TS;
};

class dq_data_ref : public dq_ref {
  public:
    dq_data_ref(mfc_t MFCtr, int mfrow, int Qrow_in, int nrows_in );
    void dq_data_ref::append_rows( int nrows );
    mfc_t MFCtr_start, MFCtr_next;
    int row_start, row_next;
    int Qrow;
    int n_rows;
};

/* Semantics of Data_Queue
   Data_Queue.first, .last are indices into row and range from
     [0..total_Qrows)
   .first is where the next row will be read from
   .last is where the next row will be written to
   first==last means either full or empty, depending on the value of full.
   
   if collection is true, then allocate_rows will throw rather than block
*/
void tminitfunc();
class data_queue {
  public:
    data_queue( int n_Qrows, int low_water );
    void init(); // allocate space for the queue
    //void operate(); // event loop
    //void tminitfunc();

  protected:
    int allocate_rows(unsigned char **rowp);
    void commit_rows( mfc_t MFCtr, int mfrow, int n_rows );
    void commit_tstamp( mfc_t MFCtr, time_t time );
    void retire_rows( dq_data_ref *dqd, int n_rows );
    void retire_tstamp( dq_tstamp_ref *dqts );
    virtual void lock();
    virtual void unlock();

    unsigned char *raw;
    unsigned char **row;
    tm_hdrw_t output_tm_type;
    int total_Qrows;
    int nbQrow; // may differ from nbrow if stripping MFCtr & Synch
    int nbDataHdr;
    int first;
    int last;
    bool full;
    
    dq_ref *first_dqr;
    dq_ref *last_dqr;
    int dq_low_water;
};

#endif
