#ifndef MEERSTETTER_H_INCLUDED
#define MEERSTETTER_H_INCLUDED
#include <stdint.h>

#ifndef ME_MAX_DRIVES
#define ME_MAX_DRIVES 8
#endif

typedef struct __attribute__((packed)) {
  int32_t  DeviceStatus;
  int32_t  ErrorNumber;
  int32_t  ErrorInstance;
  int32_t  ErrorParameter;
  float    ObjectTemp;
  float    SinkTemp;
  float    TargetObjectTemp;
  float    ActualOutputCurrent;
  float    ActualOutputVoltage;
  /**
   * Mask is bit-mapped to indicate whether we received a
   * response for each of the items above. Each bit is
   * cleared if the corresponding value was received.
   * Bit 0 is reserved to indicate that all values were
   * received. If a readback is outstanding an TM sync,
   * the drive Stale count is incremented.
   *  Bit 0: All values responding
   *  Bit 1: DeviceStatus
   *  Bit 2: ErrorNumber
   *  Bit 3: ErrorInstance
   *  Bit 4: Error Parameter
   *  Bit 5: ObjectTemp
   *  Bit 6: SinkTemp
   *  Bit 7: TargetObjectTemp (SetPoint)
   *  Bit 8: ActualOutputCurrent
   *  Bit 9: ActualOutputVoltage
   */
  uint16_t Mask;
  uint16_t Stale;
} me_drive_t;

typedef struct __attribute__((packed)) {
  // uint32_t me_status;
  me_drive_t drive[ME_MAX_DRIVES];
} meerstetter_t;

#endif
