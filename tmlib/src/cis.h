#ifndef CIS_H_INCLUDED
#define CIS_H_INCLUDED
// This is the internal header file for cis.c
#include "cmdalgo.h" // defines IOFUNC_ATTR_T

/* These defs must preceed the following includes: */
struct ocb;
struct ioattr_s;
// Define IOFUNC_OCB_T to our local definition.
// RESMGR_OCB_T gets defined to IOFUNC_OCB_T in <sys/iofunc.h>
#define IOFUNC_OCB_T struct ocb
#define IOFUNC_ATTR_T struct ioattr_s
#define THREAD_POOL_PARAM_T dispatch_context_t

#include <sys/iofunc.h>
#include <sys/dispatch.h>

#define CMD_MAX_COMMAND_OUT 160 // Maximum command message output length
#define CMD_MAX_COMMAND_IN 300  // Maximum command message input length

typedef struct command_out_s {
  struct command_out_s *next;
  int ref_count;
  char command[CMD_MAX_COMMAND_OUT];
  int cmdlen;
} command_out_t;

typedef struct ocb {
  iofunc_ocb_t hdr;
  command_out_t *next_command;
  struct ocb *next_ocb; // for blocked list
  int rcvid;
  int nbytes_requested;
} ocb_t;

/* IOFUNC_ATTR_T extended to provide blocked-ocb-list
   for each mountpoint */
typedef struct ioattr_s {
  iofunc_attr_t attr;
  iofunc_notify_t notify[3]; /* notification list used by iofunc_notify*() */
  ocb_t *blocked;
  command_out_t *first_cmd;
  command_out_t *last_cmd;
  struct ioattr_s *next;
  char *nodename;
} ioattr_t;


extern IOFUNC_ATTR_T *cis_setup_rdr( char *node );
extern void cis_turf( IOFUNC_ATTR_T *handle, char *format, ... );

#endif /* CIS_H_INCLUDED */
