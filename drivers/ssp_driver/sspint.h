/* sspint.h: Internal defintions for sspdrv */
#ifndef SSPINT_H_INCLUDED
#define SSP_INT_H_INCLUDED

#include "sspdrv.h"
#include "mlf.h"
enum fdstate { FD_IDLE, FD_READ, FD_WRITE };
extern enum fdstate tcp_state, udp_state;

extern void sspdrv_init(int argc, char **argv);
extern int tcp_create( int board_id );
extern void tcp_enqueue( char *cmd );
extern int tcp_send(void);
extern int tcp_recv(void);
extern int udp_create(void);
extern int udp_receive(long int *scan, size_t length );
extern void udp_close(void);

typedef struct {
	unsigned short NS, NA, NC;
	unsigned short NE;
  unsigned short NF; /* Frequency Divisor */
	int NP; /* udp port number */
} ssp_config_t;
extern ssp_config_t ssp_config;
extern ssp_data_t ssp_data;

#endif
