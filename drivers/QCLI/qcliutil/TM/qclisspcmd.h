#ifndef QCLISSPCMD_H_INCLUDED
#define QCLISSPCMD_H_INCLUDED

#include "hsatod.h"

typedef struct {
  hsatod_setup_t *setup;
  cmdif_rd *if_ssp;
} qclissp_t;

#endif
