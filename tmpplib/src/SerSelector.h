#ifndef SERSELECTOR_H_INCLUDED
#define SERSELECTOR_H_INCLUDED
/* SerSelector.h */

#include "Selector.h"
#include "collect.h"
#include "tm.h"

/**
 * A Selectee wrapper for Col_send() and friends to support sending data to
 * collection. The data structure should be defined in a header file shared
 * with collection and declared in tmc as 'TM "Receive" <name> 1'. The 1
 * indicates that data will be reported synchronously to telemetry.
 * The intention is that the same data structure will be shared with
 * another Selectee (possibly one derived from Ser_Sel) to record data
 * as it is received.
 *
 * Whenever the data is written to telemetry, Selector::set_gflag(0)
 * is called. Another Selectee can request to be notified of this by
 * setting bit Selector::gflag(0) in their flags word.
 */
class TM_Selectee : public Selectee {
  public:
    TM_Selectee( const char *name, void *data, unsigned short size );
    ~TM_Selectee();
    int ProcessData(int flag);
  private:
    send_id TMid;
};

/**
 * A Selectee to monitor a command channel. The default channel is
 * "cmd/Quit" and the default action is to terminate the event loop,
 * but this can be overridden in a subclass.
 */
class Cmd_Selectee : public Selectee {
  public:
    Cmd_Selectee( const char *name = "cmd/Quit" );
    int ProcessData(int flag);
};

/**
 * A Selectee for monitoring a serial line.
 */
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
