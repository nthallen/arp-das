#ifndef MEERSTETTER_H_INCLUDED
#define MEERSTETTER_H_INCLUDED
#include <stdint.h>

#ifndef ME_MAX_DRIVES
#define ME_MAX_DRIVES 2
#endif

typedef struct __attribute__((packed)) {
  int32_t  DeviceStatus;
  float    ObjectTemp;
  float    SinkTemp;
  float    TargetObjectTemp;
  uint16_t Mask;
  uint16_t Stale;
} me_drive_t;

typedef struct __attribute__((packed)) {
  uint32_t me_status;
  me_drive_t drive[ME_MAX_DRIVES];
} meerstetter_t;

#endif
