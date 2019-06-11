#ifndef MEERSTETTER_H_INCLUDED
#define MEERSTETTER_H_INCLUDED
#include <stdint.h>

typedef struct __attribute__((packed)) {
  sometype DeviceStatus;
	sometype ObjectTemp;
	sometype SinkTemp;
	sometype TargetObjectTemp;
};

#endif
