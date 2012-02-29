#ifndef SERSELECTOR_H_INCLUDED
#define SERSELECTOR_H_INCLUDED
/* SerSelector.h */

#include "Selector.h"
#include "collect.h"
#include "tm.h"

class TM_Selectee : public Selectee {
  public:
    TM_Selectee( const char *name, void *data, unsigned short size );
    ~TM_Selectee();
    int ProcessData(int flag);
  private:
    send_id TMid;
};

class Cmd_Selectee : public Selectee {
  public:
    Cmd_Selectee( const char *name = "cmd/Quit" );
    int ProcessData(int flag);
};

class Ser_Sel : public Selectee {
  public:
    Ser_Sel(const char *path, int open_flags, int bufsz);
    ~Ser_Sel();
    void setup( int baud, int bits, char par, int stopbits,
		int min, int time );
  protected:
    int fillbuf();
    void consume(int nchars);
    void report_err( const char *msg, ... );
    void report_ok();
    int not_found( char c );
    int not_int( int &val );
    int not_str( const char *str );
    int not_float( float &val );
    int nc, cp;
    char *buf;
  private:
    int bufsize;
    int n_fills, n_empties;
    /** Number of qualified errors. Decremented by report_ok() */
    int n_errors;
    /** Number of messages currently suppressed. */
    int n_suppressed;
    /** Total number of errors found. */
    int total_errors;
    int total_suppressed;
};

#endif
