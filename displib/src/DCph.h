#ifndef DCPH_H_INCLUDED
#define DCPH_H_INCLUDED
#include "DC.h"

class ph_data_client : public data_client {
  public:
    ph_data_client( int bufsize_in, int fast = 1, int non_block = 1 );
    void operate();
    int read();
};

#endif
