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

#define MKS_STAT_C   0x0001
#define MKS_STAT_CR  0x0002
#define MKS_STAT_E   0x0004
#define MKS_STAT_H   0x0008
#define MKS_STAT_HH  0x0010
#define MKS_STAT_IP  0x0020
#define MKS_STAT_L   0x0040
#define MKS_STAT_LL  0x0080
#define MKS_STAT_M   0x0100
#define MKS_STAT_OC  0x0200
#define MKS_STAT_P   0x0400
#define MKS_STAT_T   0x0800
#define MKS_STAT_U   0x1000
#define MKS_STAT_V   0x2000

typedef struct __attribute__((packed)) {
  mks_drive_t drive[MKS_MAX_DRIVES];
} mksflow_t;

#endif
