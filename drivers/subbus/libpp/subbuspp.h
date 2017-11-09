/** @file
 subbuspp.h defines the interface to the subbus resident library
 Before calling the subbus routines, you must first call
 load_subbus().  This returns the subfunction of the resident
 subbus library or 0 if none is installed.
 */
#ifndef SUBBUSPP_H_INCLUDED
#define SUBBUSPP_H_INCLUDED
#include <sys/siginfo.h>
#include <stdint.h>
#include "subbus.h"
#include "subbusd.h"

#define SUBBUS_VERSION 0x501 /* subbus version 5.01 QNX6 */

class subbuspp {
public:
  subbuspp(const char *name);
  ~subbusp();
  int load();
  const char *get_subbus_name();
  int write_ack(uint16_t addr, uint16_t data);
  int read_ack(uint16_t addr, uint16_t *data);
  uint16_t read_subbus(uint16_t addr);
  uint16_t cache_read(uint16_t addr);
  int cache_write(uint16_t addr, uint16_t data);
  int inline write_subbus(uint16_t addr, uint16_t data) {
      return write_ack(addr, data);
    }
  int inline sbwr(uint16_t addr, uint16_t data) {
      return write_ack(addr, data);
    }
  uint16_t inline sbrd(uint16_t addr) { return read_subbus(addr); }
  int mread_subbus( subbus_mread_req *req, uint16_t *data);
  int mread_subbus_nw(subbus_mread_req *req, uint16_t *data,
                        uint16_t *nwords);
  subbus_mread_req *pack_mread_requests( unsigned int addr, ... );
  subbus_mread_req *pack_mread_request( int n_reads, const char *req );
  int set_cmdenbl(int value);
  int set_cmdstrobe(int value);
  uint16_t read_switches(void);
  int set_failure(uint16_t value);
  uint16_t read_failure(void);
  int  tick_sic(void);
  int disarm_sic(void);
  int cache_write(uint16_t addr, uint16_t data);
  int subbus_int_attach( char *cardID, uint16_t address,
      uint16_t region, struct sigevent *event );
  int subbus_int_detach( char *cardID );
  int subbus_quit(void);

private:
  int send_to_subbusd( uint16_t command, void *data,
		int data_size, uint16_t exp_type );
  int send_CSF( uint16_t command, uint16_t val );
  uint16_t read_special( uint16_t command );
  subbus_mread_req *pack_mread( int req_len, int n_reads, const char *req_str );
  
  const char *path;
  int sb_fd;
  const uint16_t subbus_version = SUBBUS_VERSION;
  uint16_t subbus_subfunction; // undefined until initialization
  uint16_t subbus_features; // ditto
  char local_subbus_name[SUBBUS_NAME_MAX];
  iov_t sb_iov[3];
  subbusd_req_hdr_t sb_req_hdr;
  subbusd_rep_t sb_reply;
};

#endif
