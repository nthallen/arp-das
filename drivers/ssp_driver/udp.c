#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "nortlib.h"
#include "nl_assert.h"
#include "sspint.h"

static int udp_socket;
enum fd_state tcp_state = FD_IDLE;

int udp_create(void) {
  struct sockaddr_in servAddr;
  int port, rc;
  socklen_t servAddr_len = sizeof(servAddr);

  /* socket creation */
  nl_assert( udp_state == FD_IDLE );
  udp_socket=socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket<0)
    nl_error( 3, "Cannot open UDP socket: %s", strerror(errno) );

  /* bind local server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(0);
  rc = bind (udp_socket, (struct sockaddr *) &servAddr,sizeof(servAddr));
  if(rc<0) nl_error( 3, "cannot bind port number %d \n",  0);
  rc = getsockname( udp_socket, (struct sockaddr *)&servAddr,
    &servAddr_len);
  port = ntohs(servAddr.sin_port);
  udp_state = FD_READ;
  return port;
}

int udp_receive(long int *scan, size_t length ) {
  struct sockaddr_in cliAddr;
  int n, cliLen;

  cliLen = sizeof(cliAddr);
  n = recvfrom(udp_socket, scan, length, 0, 
   (struct sockaddr *) &cliAddr, &cliLen);
  return n;
}

void udp_close(void) {
  if ( udp_state != FD_IDLE ) {
    close(udp_socket);
    udp_state = FD_IDLE;
  }
}
