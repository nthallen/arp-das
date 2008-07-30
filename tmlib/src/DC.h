#ifndef DC_H_INCLUDED
#define DC_H_INCLUDED
#include "tm.h"

class data_client {
  public:
    data_client(int bufsize_in, int fast = 0, int non_block = 0);
    data_client(int bufsize_in, int non_block, char *srcfile);
    void operate(); // event loop
    static unsigned int next_minor_frame, majf_row, minf_row;
  protected:
    virtual void process_data();
    virtual void process_init();
    virtual void process_tstamp();
    int bfr_fd;
    void read();
    bool quit;
    tm_msg_t *msg;
    int nbQrow; // may differ from nbrow if stripping MFCtr & Synch
    int nbDataHdr;
    bool tm_info_ready;
    tm_hdrw_t output_tm_type;
  private:
    void process_message();
    int nQrows;
    int bufsize;
    unsigned int bytes_read; /// number of bytes currently in buf
    unsigned int toread; /// number of bytes needed before next action
    char *buf;
    void init(int bufsize_in, int non_block, char *srcfile);
};

void tminitfunc();


#endif
