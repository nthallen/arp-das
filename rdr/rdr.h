#ifndef RDR_H_INCLUDED
#define RDR_H_INCLUDED

#include <pthread.h>
#include <semaphore.h>
#include "mlf.h"

#ifdef __cplusplus
#include "DGcol.h"
#include "DC.h"

class Rdr_quit_pulse;

class Reader : public data_generator, public data_client {
  public:
    Reader(int nQrows, int low_water, int bufsize, const char *path);
    void event(enum dg_event evt);
    void *input_thread();
    void *output_thread();
    void control_loop();
    void service_row_timer();
  protected:
    void process_tstamp();
    void process_data();
    int  process_eof();
    void lock();
    void unlock();
    int it_blocked;
    sem_t it_sem;
    int ot_blocked;
    sem_t ot_sem;
    pthread_mutex_t dq_mutex;
    bool have_tstamp;
  private:
    mlf_def_t *mlf;
    Rdr_quit_pulse *RQP;
};

class Rdr_quit_pulse : public DG_dispatch_client {
  public:
    Rdr_quit_pulse(Reader *rdr_ptr);
    ~Rdr_quit_pulse();
    void pulse();
    void attach();
    int ready_to_quit();
    Reader *rdr;
  private:
    int pulse_code;
    int coid;
};

extern "C" {
#endif

  void *input_thread(void *Reader_ptr);
  void *output_thread(void *Reader_ptr);
  void rdr_init( int argc, char **argv );

#ifdef __cplusplus
};
#endif

#define OT_BLOCKED_STOPPED 1
#define OT_BLOCKED_TIME 2
#define OT_BLOCKED_DATA 3
#define IT_BLOCKED_DATA 1
#define IT_BLOCKED_EOF 2

#endif
