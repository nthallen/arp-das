/*
  mamba_temp.cc
  
  read MSR during ISR and queue data to thread
*/
#include <time.h>
#include <stdint.h>
#include <unistd.h> // for delay()
#include <atomic.h>
#include <sys/neutrino.h>
#include "nortlib.h"

//CPU MSR definitions
#define IA32_THERM_STS_MSR          0x19C
#define EXT_CONFIG_MSR              0x0EE
#define IA32_THERM_RD_VALID         0x80000000
#define IA32_TJ90                   0x40000000
#define IA32_THERM_STS_DIGRD        0x007F0000
#define IA32_THERM_STS_FLAGS        0x00000140

#define MSR_READ_REQ 1

typedef struct {
  volatile uint32_t MSR_id;
  volatile uint64_t MSR_val;
  volatile unsigned count;
  volatile unsigned flags;
} MSR_data;

const struct sigevent *RdMsrHandler(void *area, int id) {
  MSR_data *MSR_p = (MSR_data *)area;
  
  atomic_add(&MSR_p->count,1);
  if (MSR_p->flags | MSR_READ_REQ) {
    atomic_clr(&MSR_p->flags,MSR_READ_REQ);
  }
  return NULL;
}

uint32_t ReadMsr(uint32_t MSR_id) {
  MSR_data MSR_d;
  int Int_id;
  MSR_d.MSR_id = MSR_id;
  MSR_d.count = 0;
  MSR_d.flags = MSR_READ_REQ;
  MSR_d.MSR_val = 0;
  Int_id = InterruptAttach(0, &RdMsrHandler, &MSR_d, sizeof(MSR_d),
      _NTO_INTR_FLAGS_END | _NTO_INTR_FLAGS_TRK_MSK);
  if (Int_id == -1)
    nl_error(3, "Error attaching clock interrupt");
  delay(1);
  if (InterruptDetach(Int_id) == -1)
    nl_error(2, "Error calling InterruptDetach");
  if (MSR_d.flags & MSR_READ_REQ)
    nl_error(2, "ReadMsr failed");
  nl_error(0, "ReadMsr: count = %u", MSR_d.count);
  return (uint32_t)MSR_d.MSR_val;
}

int main(int argc, char **argv) {
  struct timespec res;
  if (clock_getres(CLOCK_REALTIME, &res))
    nl_error(3, "clock_getres() returned an error");
  if (res.tv_sec)
    nl_error(1, "Clock resolution is long!: %ld sec", res.tv_sec);
  nl_error(0, "Clock resolution is %ld ns", res.tv_nsec);
  if (ThreadCtl( _NTO_TCTL_IO, 0 ) == -1)
    nl_error(3, "Error calling ThreadCtl()");
  ReadMsr(EXT_CONFIG_MSR);
  return 0;
}
