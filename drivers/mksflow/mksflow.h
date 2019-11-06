#ifndef MKSFLOW_H_INCLUDED
#define MKSFLOW_H_INCLUDED
#include <stdint.h>

#ifndef MKS_MAX_DRIVES
#define MKS_MAX_DRIVES 2
#endif

typedef struct __attribute__((packed)) {
  float    Flow;         // FX
  float    FlowSetPoint; // SX
  float    DeviceTemp;   // TA
  uint16_t DeviceStatus; // T
  uint8_t  ACK;
  uint8_t  Stale;
} mks_drive_t;

typedef struct __attribute__((packed)) {
  mks_drive_t drive[MKS_MAX_DRIVES];
} mksflow_t;

#endif
