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
enum fdstate { FD_IDLE, FD_READ, FD_WRITE, FD_CONNECT };
extern enum fdstate tcp_state, udp_state;

extern int tcp_socket;
extern void sspdrv_init(const char *name, int argc, char * const *argv);
extern int tcp_create(const char *hostname);
extern void tcp_reset(const char *hostname);
extern void tcp_enqueue( char *cmd );
extern int tcp_send(void);
extern int tcp_recv(void);
extern void tcp_connected(void);
extern int udp_create(const char *interface, const char *portspec);
extern int udp_socket;
extern int udp_receive(long int *scan, size_t length );
extern void udp_close(void);
extern void udp_read(mlf_def_t *mlf, int do_amp);

#define RECV_BUF_SIZE SSP_MAX_CTRL_MSG

typedef struct {
	unsigned short NS, NA, NC;
	unsigned short NE;
  unsigned short NF; /* Frequency Divisor */
	int NP; /* udp port number */
  int LE; /* logging enabled */
} ssp_config_t;
extern ssp_config_t ssp_config;
extern ssp_data_t ssp_data;
extern ssp_amp_data_t ssp_amp_data;

typedef struct {
  unsigned short NZ, NN, NM, NSamp;
  float meanX, sumX2;
  int modified;
} noise_config_t;

extern noise_config_t noise_config;

#endif
