/* sspdrv.h
   structure definition for telemetry
  */
#ifndef SSPDRV_H_INCLUDED
#define SSPDRV_H_INCLUDED

typedef struct {
  unsigned long index;
  unsigned long ScanNum;
  unsigned long Total_Skip;
  unsigned short Flags;
  unsigned char Status;
  // unsigned short NChannels;
  // unsigned short NSamples;
  // unsigned short NCoadd;
  // unsigned short NAvg;
  // unsigned short NSkL;
  // unsigned short NSkP;
  // unsigned long Spare;
} __attribute__((packed)) ssp_data_t;

#define SSP_OVF_MASK 0x01FF
#define SSP_CAOVF(x) ((x)&7)
#define SSP_PAOVF(x) (((x)>>3)&7)
#define SSP_ADOOR(x) (((x)>>6)&7)

#define SSP_STATUS_GONE 0
#define SSP_STATUS_CONNECT 1
#define SSP_STATUS_READY 2
#define SSP_STATUS_ARMED 3
#define SSP_STATUS_TRIG 4

/*
It could be that Flags (or something derived from flags) and Total_Skip will be sufficient for telemetry.
*/

#endif
