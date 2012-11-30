#ifndef HSATOD_H
#define HSATOD_H

/* For QNX6, we don't need to establish a special interface to the driver.
   The driver will read ASCII commands from the appropriate interface of
   the form /dev/huarp/$Experiment/cmd/$Board. Instead of transmitting
   the hsatod_setup_t struct, we will convert to ASCII However, I need
   to provide the struct typedef, because that is how qclicomp
   communicates with the command server.

   Since the communication to the server is handled by cis_turf(), all
   we need to do is provide the appropriate command strings.
 */

typedef struct {
  unsigned long FSample;
  unsigned long NSample;
  unsigned long NReport;
  unsigned long NAvg;
  unsigned short NCoadd;
  unsigned short FTrigger;
  unsigned short Options;
  unsigned long TzSamples;
} hsatod_setup_t;

#define HSAD_OPT_A 1
#define HSAD_OPT_B 2
#define HSAD_OPT_C 4
#define HSAD_TRIG_0 0
#define HSAD_TRIG_1 8
#define HSAD_TRIG_2 0x10
#define HSAD_TRIG_3 0x18
#define HSAD_TRIG_RISING 0x20
#define HSAD_TRIG_AUTO 0x40
#define HSAD_RINGDOWN 0x80

#endif

