#ifndef RDR_H_INCLUDED
#define RDR_H_INCLUDED

#include <pthread.h>
#include <semaphore.h>
#include "DG_col.h"

class DG_rdr;

class Reader : public data_generator, public data_client {
  public:
    Reader(int nQrows, int low_water, int bufsize);
    void event(enum dg_event evt);
    void *input_thread();
    void *output_thread();
    void it_operate();
  protected:
    void process_tstamp();
    void process_data();
    virtual void lock();
    virtual void unlock();
    int it_blocked;
    sem_t it_sem;
    int ot_blocked;
    sem_t ot_sem;
    pthread_mutex_t dq_mutex;
    bool have_tstamp;
};

extern "C" {
  void *input_thread(void *Reader_ptr);
  void *output_thread(void *Reader_ptr);
};

#define OT_BLOCKED_STOPPED 1
#define OT_BLOCKED_TIME 2
#define OT_BLOCKED_DATA 3
#define IT_BLOCKED_DATA 1

#endif
