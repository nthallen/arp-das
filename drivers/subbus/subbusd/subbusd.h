#ifndef SUBBUSD_H_INCLUDED
#define SUBBUSD_H_INCLUDED
#include <sys/iomsg.h>
#include <sys/siginfo.h>

#define SUBBUSD_MGRID_OFFSET 1
#define SUBBUSD_MGRID (_IOMGR_PRIVATE_BASE + SUBBUSD_MGRID_OFFSET)

#define SUBBUS_NAME_MAX 80
#define CardID_MAX 32

/*
 * Limit definition to what is strictly required
 * Convenience functions will remain in the library.
 * Interface between the library and the driver will
 * use a structure defined here. Encoding into ASCII
 * will only happen when translating to a serial
 * platform.
 */

typedef struct __attribute__((__packed__)) {
  struct _io_msg iohdr;
  unsigned short sb_kw;
  unsigned short command;
} subbusd_req_hdr_t;

/* sb_kw value */
#define SB_KW 0x7362

/* command values */
#define SBC_READACK 0
#define SBC_WRITEACK 1
#define SBC_SETCMDENBL 2
#define SBC_SETCMDSTRB 3
#define SBC_READSW 4
#define SBC_SETFAIL 5
#define SBC_READFAIL 6
#define SBC_TICK 7
#define SBC_DISARM 8
#define SBC_GETCAPS 9
#define SBC_INTATT 10
#define SBC_INTDET 11
#define SBC_READCACHE 12
#define SBC_WRITECACHE 13
#define SBC_QUIT 14

typedef struct __attribute__((__packed__)) {
  unsigned short address;
  unsigned short data;
} subbusd_req_data0;

typedef struct __attribute__((__packed__)) {
  unsigned short data;
} subbusd_req_data1;

typedef struct __attribute__((__packed__)) {
  char cardID[CardID_MAX];
  unsigned short address;
  unsigned short region;
  struct sigevent event;
} subbusd_req_data2;

typedef struct __attribute__((__packed__)) {
  char cardID[CardID_MAX];
} subbusd_req_data3;

typedef struct __attribute__((__packed__)) {
  unsigned short n_reads;
  char multread_cmd[256];
} subbusd_req_data4;

typedef struct __attribute__((__packed__)) {
  subbusd_req_hdr_t sbhdr;
  union {
    subbusd_req_data0 d0;
    subbusd_req_data1 d1;
    subbusd_req_data2 d2;
    subbusd_req_data3 d3;
    subbusd_req_data4 d4;
  } data;
} subbusd_req_t;

typedef struct __attribute__((__packed__)) {
  unsigned short subfunc;
  unsigned short features;
  char name[SUBBUS_NAME_MAX];
} subbusd_cap_t;

#define SB_MAX_MREAD 256
/**
 * We need one or two shorts per read, depending on
 * whether we want acknowledge info or not.
 */
typedef struct __attribute__((__packed__)) {
  unsigned short rvals[2*SB_MAX_MREAD];
} subbusd_mread_t;

typedef struct __attribute__((__packed__)) {
  signed short status;
  unsigned short ret_type;
} subbusd_rep_hdr_t;

typedef struct __attribute__((__packed__)) {
  subbusd_rep_hdr_t hdr;
  union {
    unsigned short value;
    subbusd_cap_t capabilities;
    subbusd_mread_t mread;
  } data;
} subbusd_rep_t;

/* status values. Status values less than zero are errors */
#define SBS_OK 0
#define SBS_ACK 1
#define SBS_NOACK 2

/* ret_type values */
#define SBRT_NONE 0 // Just return the status
#define SBRT_US 1   // Return unsigned short value
#define SBRT_CAP 2  // Capabilities
#define SBRT_MREAD 3 // Multi-Read
#define SBRT_MREADACK 4 // Multi-Read w/ACK


#endif
