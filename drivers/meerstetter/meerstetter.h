#ifndef MEERSTETTER_H_INCLUDED
#define MEERSTETTER_H_INCLUDED
#include <stdint.h>

typedef struct __attribute__((packed)) {
  int32_t DeviceStatus;
  float   ObjectTemp;
  float   SinkTemp;
  float   TargetObjectTemp;
} meerstetter_t;

extern meerstetter_t meerstetter;

#endif
