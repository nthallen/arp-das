/** \file DC.h
 * Data Client Clases
 */
#ifndef DC_H_INCLUDED
#define DC_H_INCLUDED
#include <stdio.h>
#include "tm.h"

#ifdef __cplusplus

/**
 * \brief Defines interface for data client connection to TMbfr
 */
class data_client {
  public:
    data_client(int bufsize_in, int fast = 0, int non_block = 0);
    data_client(int bufsize_in, int non_block, char *srcfile);
    void operate(); // event loop
    void resize_buffer(int bufsize_in);
    void load_tmdac(char *path);
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
    virtual const char *context();
    void dc_init();
    void seek_tmid();
    tm_msg_t *msg;
    int nbQrow; // may differ from nbrow if stripping MFCtr & Synch
    int nbDataHdr;
    tm_hdrw_t input_tm_type;
    void init_tm_type();
  private:
    void process_message();
    int nQrows;
    int bufsize;
    int dc_state;
    unsigned int bytes_read; /// number of bytes currently in buf
    unsigned int toread; /// number of bytes needed before next action
    bool tm_info_ready;
    char *buf;
    void init(int bufsize_in, int non_block, const char *srcfile);
};

#define DC_STATE_HDR 0
#define DC_STATE_DATA 1

class ext_data_client : public data_client {
  public:
    inline ext_data_client(int bufsize_in, int fast = 0, int non_block = 0) :
      data_client(bufsize_in, fast, non_block) {}
  protected:
    void process_data();
};

extern void tminitfunc();

extern "C" {
#endif

/* This is all that is exposed to a C module */
extern void dc_set_srcnode(char *nodename);

#ifdef __cplusplus
};
#endif

#endif
