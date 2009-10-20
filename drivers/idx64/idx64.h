/* idx64.h Defines interface to the idx64 driver.
 *
 * &idx64_cmds
 *	: Drive &drive &direction &steps *
 *	: Scan &drive &direction &steps by %d (Enter Steps per Step) *
 *	: Stop &drive *
 *	: Drive &drive Online *
 *	: Drive &drive Offline *
 *	: Set &chop_drive Online Position %d (Enter Online Position) *
 *	: Set &chop_drive Online Delta
 *	   %d (Enter positive number of steps between dithered online positions) *
 *	: Set &chop_drive Offline Delta
 *	   %d (Enter signed number of steps from Online to Offline position) *
 *	: Move &chop_drive Online Position Out *
 *	: Move &chop_drive Online Position In *
 *	;
 * &drive <byte_t>
 *	: &chop_drive
 *	: Bellows
 *	: Primary Duct Throttle
 *	: Secondary Duct Throttle
 *	;
 * &chop_drive <byte_t>
 *	: Etalon
 *	: Attenuator
 *	;
 * &direction <byte_t>
 *	: In { $0 = IX64_IN; }
 *	: Out { $0 = IX64_OUT; }
 *	: To { $0 = IX64_TO; }
 *	;
 * &steps <step_t>
 *	: %d (Enter Number of Steps or Step Number) { $0 = $1; }
 *	;
 *
 */
#ifndef IDX64_H_INCLUDED
#define IDX64_H_INCLUDED

typedef unsigned char byte_t;
typedef unsigned short step_t;
typedef struct {
  byte_t dir_scan; /* scan/drive and direction */
  byte_t drive;    /* which drive */
  step_t steps;     /* number of steps or final step */
  step_t dsteps;    /* steps per scan */
} idx64_cmnd;

#define IX64_IN 0
#define IX64_OUT 1
#define IX64_TO 2
/* Dir is the combination of the in/out/to values for masking */
#define IX64_DIR 3
#define IX64_SCAN 4
/* All following commands don't use the dir/scan bits */
#define IX64_STOP 8
#define IX64_ONLINE 9
#define IX64_OFFLINE 10
#define IX64_ALTLINE 11
#define IX64_PRESET_POS 12
#define IX64_QUIT 13
#define IX64_MOVE_ONLINE_OUT 14
#define IX64_MOVE_ONLINE_IN 15
#define IX64_SET_ONLINE 16
#define IX64_SET_ON_DELTA 17
#define IX64_SET_OFF_DELTA 18
#define IX64_SET_ALT_DELTA 19
#define IX64_SET_SPEED 20
#define IX64_SET_OFF_POS 21
#define IX64_SET_ALT_POS 22
#define IX64_SET_HYSTERESIS 23

typedef struct {
  unsigned short type; /* 'ix' */
  idx64_cmnd ix;
} idx64_msg;

typedef struct {
  short status;
} idx64_reply;

#define IDX64_MSG_TYPE 'ix'
#define INDEXER_PROXY_ID 2
#define IDX64_NAME "idx64"

/* These are definitions for "flag" values extracted from
   IXStt
*/
#define IXFLAG_SCAN 1
#define IXFLAG_ONLINE 2
#define IXFLAG_OFFLINE 4
#define IXFLAG_ALTLINE 6

/* Functions return:
   0 on success, -1 on error with errno set appropriately
*/
int idx64_cmd(byte_t cmd, byte_t drive, step_t steps, step_t dsteps);
int idx64_drive(byte_t drive, byte_t dir, step_t steps);
int idx64_scan(byte_t drive, byte_t dir, step_t steps, step_t dsteps);
#define idx64_drive(drv,dir,s) idx64_cmd((dir&IX64_DIR), drv, s, 0)
#define idx64_scan(drv,dir,st,ds) idx64_cmd((dir&IX64_DIR)|IX64_SCAN,drv,st,ds)
#define idx64_stop(drive) idx64_cmd(IX64_STOP, drive, 0, 0)
#define idx64_online(drive) idx64_cmd(IX64_ONLINE, drive, 0, 0)
#define idx64_offline(drive) idx64_cmd(IX64_OFFLINE, drive, 0, 0)
#define idx64_altline(drive) idx64_cmd(IX64_ALTLINE, drive, 0, 0)
#define idx64_preset(drv, stps) idx64_cmd(IX64_PRESET_POS, drv, stps, 0)
#define idx64_move_in(drive) idx64_cmd(IX64_MOVE_ONLINE_IN, drive, 0, 0)
#define idx64_move_out(drive) idx64_cmd(IX64_MOVE_ONLINE_OUT, drive, 0, 0)
#define idx64_set_online(d, s) idx64_cmd(IX64_SET_ONLINE, d, s, 0)
#define idx64_online_delta(d, s) idx64_cmd(IX64_SET_ON_DELTA, d, s, 0)
#define idx64_offline_delta(d, s) idx64_cmd(IX64_SET_OFF_DELTA, d, s, 0)
#define idx64_offline_pos(d, s) idx64_cmd(IX64_SET_OFF_POS, d, s, 0)
#define idx64_altline_delta(d, s) idx64_cmd(IX64_SET_ALT_DELTA, d, s, 0)
#define idx64_altline_pos(d, s) idx64_cmd(IX64_SET_ALT_POS, d, s, 0)
#define idx64_hysteresis(d, s) idx64_cmd(IX64_SET_HYSTERESIS, d, s, 0)
#define idx64_speed(drv,s) idx64_cmd(IX64_SET_SPEED, drv, s, 0)
#endif

/*
    AL IX64_ALTLINE 1
    AP IX64_SET_ALT_POS 2
    AD IX64_SET_ALT_DELTA 2
    DI IX64_IN 2
    DO IX64_OUT 2
    DT IX64_TO 2
    FD IX64_SET_OFF_DELTA 2
    FP IX64_SET_OFF_POS 2
    HY IX64_SET_HYSTERESIS 2
    MI IX64_ONLINE_IN 1
    MO IX64_ONLINE_OUT 1
    ON IX64_ONLINE 1
    OF IX64_OFFLINE 1
    OP IX64_SET_ONLINE 2
    OD IX64_SET_ON_DELTA 2
    PR IX64_PRESET_POS 2
    QU IX64_QUIT 0
    SI (IX64_SCAN|IX64_IN) 3
    SO (IX64_SCAN|IX64_OUT) 3
    SP IX64_STOP 1
    SS IX64_SET_SPEED 2
    ST (IX64_SCAN|IX64_TO) 3

    CC[\d+[:\d+[:\d+]]]\n
*/