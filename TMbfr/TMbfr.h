/* Copyright 2001 by the President and Fellows of Harvard College */
#ifndef DRBFR_H
#define DRBFR_H

#define DRBFR_NPARTS_MAX 5
#define DRBFR_MSG_MAX 16384

#define THREAD_POOL_PARAM_T dispatch_context_t
#define IOFUNC_OCB_T struct ocb
#include <sys/iofunc.h>
#include <sys/dispatch.h>

/* I have group related members into structs here purely
   to help make clear which members are related.
   If you don't like this approach, let me know.
*/
typedef struct ocb {
  iofunc_ocb_t hdr;
  struct {
	char *buf;
	int nb, off;
  } part;
  struct {
	int rcvid;
	int nbytes;
  } read;
  int rows_missing;
  int hold_index;
} ocb_t;
#endif
