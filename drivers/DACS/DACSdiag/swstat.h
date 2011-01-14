#ifndef SWSTAT_H_INCLUDED
#define SWSTAT_H_INCLUDED

typedef struct {
  unsigned short SWStat;
} swstat_t;
extern swstat_t SWData;

#define SWS_OK 0
#define SWS_IDX_TEST0 1

#endif
