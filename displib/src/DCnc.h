#ifndef DCNC_H_INCLUDED
#define DCNC_H_INCLUDED
#include "DC.h"

class nc_data_client : public data_client {
  public:
    nc_data_client( int bufsize_in, int fast = 1, int non_block = 1 );
    void process_data();
    void operate();
    void read();
};

#endif
