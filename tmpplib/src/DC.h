#ifndef DC_H_INCLUDED
#define DC_H_INCLUDED
#include <stdio.h>
#include "tm.h"

#ifdef __cplusplus

class data_client {
  public:
    data_client(int bufsize_in, int fast = 0, int non_block = 0);
    data_client(int bufsize_in, int non_block, char *srcfile);
    void operate(); // event loop
    void resize_buffer(int bufsize_in);
    static unsigned int next_minor_frame, majf_row, minf_row;
    static char *srcnode;
  protected:
    virtual void process_data() = 0;
    virtual void process_init();
    virtual void process_tstamp();
    virtual int process_eof();
    int bfr_fd;
    void read();
    bool dc_quit;
    tm_msg_t *msg;
    int nbQrow; // may differ from nbrow if stripping MFCtr & Synch
    int nbDataHdr;
    tm_hdrw_t input_tm_type;
    void init_tm_type();
  private:
    void process_message();
    int nQrows;
    int bufsize;
    unsigned int bytes_read; /// number of bytes currently in buf
    unsigned int toread; /// number of bytes needed before next action
    bool tm_info_ready;
    char *buf;
    void init(int bufsize_in, int non_block, char *srcfile);
};

class ext_data_client : public data_client {
  public:
    inline ext_data_client(int bufsize_in, int fast = 0, int non_block = 0) :
      data_client(bufsize_in, fast, non_block) {}
  protected:
    void process_data();
};

void tminitfunc();

extern "C" {
#endif

/* This is all that is exposed to a C module */
extern void dc_set_srcnode(char *nodename);

#ifdef __cplusplus
};
#endif

#endif
