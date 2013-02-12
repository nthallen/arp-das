/**
 * \file Selector.h
 */
#ifndef SELECTOR_H_INCLUDED
#define SELECTOR_H_INCLUDED

#include <map>
#include <time.h>
#include "Timeout.h"

class Selector;

class Selectee {
  public:
    Selectee(int fd_in, int flag);
    Selectee();
    ~Selectee();
    /**
     * @return non-zero if we should quit
     */
    virtual int ProcessData(int flag) = 0;
    virtual Timeout *GetTimeout();

    int fd;
    int flags;
    Selector *Stor;
};

typedef std::vector<Selectee *> SelecteeVec;

class Selector {
  public:
    static const int Sel_Read = 1;
    static const int Sel_Write = 2;
    static const int Sel_Except = 4;
    static const int Sel_Timeout = 8;
    Selector();
    ~Selector();
    void add_child(Selectee *P);
    void delete_child(int fd_in);
    int update_flags(int fd_in, int flag);
    void set_gflag( unsigned gflag_index );
    /**
     * Method to map a global flag number to a bit mask to be
     * set in a Selectee's flags word.
     * @param gflag_index global flag bit number
     * @return bit mask selecting the specified global flag.
     */
    static inline int gflag(unsigned gflag_index) {
      return( 1 << (gflag_index+4) );
    }
    void event_loop();
  private:
    SelecteeVec S;
    SelecteeVec::iterator find_child_by_fd(int fd);
    bool children_changed;
    int gflags;
    virtual int ProcessTimeout();
    virtual Timeout *GetTimeout();
};

#endif
