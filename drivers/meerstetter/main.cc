#include "meerstetter_int.h"
#include "meerstetter.h"
#include "msg.h"
#include "oui.h"

const char *Me_Ser_path = "/dev/ser1";
meerstetter_t meerstetter;

int main(int argc, char **argv) {
  oui_init_options(argc, argv);
  Selector S;
  Me_Ser Ser(Me_Ser_path);
  Me_Cmd Cmd(&Ser);
  TM_Selectee TM("meerstetter", &meerstetter, sizeof(meerstetter));
  S.add_child(&Ser);
  S.add_child(&Cmd);
  S.add_child(&TM);
  msg(0, "Starting: V1.0");
  S.event_loop();
  msg(0, "Terminating");
  return 0;
}

