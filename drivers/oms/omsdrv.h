/* omsdrv.h
   Header for Oregon Micro Systems Motor Controller
   Driver Library Interface
*/
#ifndef OMSDRV_H_INC
#define OMSDRV_H_INC

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned short USHRT;

/* Internal Definitions */

#define OMS_CMD_MAX 80

typedef struct {
  signed long step[4];
  unsigned char status[4];
} OMS_TM_Data;

#ifdef __cplusplus
};
#endif


#endif
