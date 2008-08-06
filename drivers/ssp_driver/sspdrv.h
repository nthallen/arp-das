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
  // unsigned short NChannels;
  // unsigned short NSamples;
  // unsigned short NCoadd;
  // unsigned short NAvg;
  // unsigned short NSkL;
  // unsigned short NSkP;
  // unsigned long Spare;
} ssp_data_t;

#define SSP_OVF_MASK 0x01FF

/*
It could be that Flags (or something derived from flags) and Total_Skip will be sufficient for telemetry.
*/

#endif
