/* sspint.h: Internal defintions for sspdrv */
#ifndef SSPINT_H_INCLUDED
#define SSP_INT_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "sspdrv.h"
#include "ssp_ad.h"
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
extern void udp_read(mlf_def_t *mlf);

#define RECV_BUF_SIZE SSP_MAX_CTRL_MSG

typedef struct {
	unsigned short NS, NA, NC;
	unsigned short NE;
  unsigned short NF; /* Frequency Divisor */
	int NP; /* udp port number */
} ssp_config_t;
extern ssp_config_t ssp_config;
extern ssp_data_t ssp_data;

#endif
