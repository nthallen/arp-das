%{
  #include "swstat.h"
  #include "collect.h"
  #include "subbus.h"
  swstat_t SWData;
  send_id SWS_id;
  #define QUIT_CMD_CLEANUP Col_send_reset(SWS_id); SWS_id = 0;
%}
