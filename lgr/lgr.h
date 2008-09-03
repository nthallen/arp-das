#ifndef LGR_H_INCLUDED
#define LGR_H_INCLUDED

#include <stdio.h>
#include "mlf.h"

#ifdef __cplusplus

  class DClgr : public data_client {
    public:
      DClgr();
      static char *mlf_config;
      static unsigned int file_limit;
    protected:
      void process_data_t1();
      void process_data_t2();
      void process_data_t3();
      void process_data();
      void process_init();
      void process_tstamp();
      mlf_def_t *mlf;
      FILE *ofp;
      int nb_out;
      void next_file();
      void write_tstamp();
      void lgr_write(void *buf, int nb, char *where);
  };

extern "C" {
#endif

  extern void lgr_init( int argc, char **argv );

#ifdef __cplusplus
};
#endif

#endif
