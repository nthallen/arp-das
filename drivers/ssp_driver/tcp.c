#include <ctype.h>
#include <sys/ioctl.h>
#include "nl_assert.h"
#include "sspint.h"

int tcp_socket;
enum fdstate tcp_state = FD_IDLE;

#define TCP_QSIZE 20
#define TCP_SEND_SIZE 30
static struct {
  int front, back, full;
  char q[TCP_QSIZE][TCP_SEND_SIZE];
} tcp_queue;

static const char *save_hostname;

/**
 * returns socket fd or dies
 */
int tcp_create( const char *hostname ) {

  int rc, nonblock = 1;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;

  save_hostname = hostname;
  h = gethostbyname(hostname);
  if(h==NULL)
    nl_error( 3, "Unknown host '%s'", hostname);

  servAddr.sin_family = h->h_addrtype;
  memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_port = htons(SSP_SERVER_PORT);

  /* create socket */
  tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (tcp_socket<0)
    nl_error( 3, "cannot open tcp_socket");

  /* bind any port number */
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(0);
  
  rc = bind(tcp_socket, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if (rc<0)
    nl_error( 3, "Cannot bind TCP port %u: %s", SSP_SERVER_PORT,
      strerror(errno));
        
  /* connect to server */
  tcp_queue.front = tcp_queue.back = 0;
  tcp_queue.full = 0;
  rc = ioctl(tcp_socket, FIONBIO, &nonblock);
  if ( rc < 0 ) nl_error( 3, "Error from ioctl(): %s", strerror(errno));
  rc = connect(tcp_socket, (struct sockaddr *) &servAddr, sizeof(servAddr));
  tcp_state = FD_CONNECT;
  ssp_data.Status = SSP_STATUS_CONNECT;
  if ( rc == 0 ) {
    tcp_state = FD_IDLE;
    ssp_data.Status = SSP_STATUS_READY;
  } else if ( rc < 0 ) {
    if ( errno != EINPROGRESS )
      nl_error( 2, "Error on connect(): %s", strerror(errno) );
  } else nl_error( 4, "Unknown response from connect: %d", rc );
  return tcp_socket;
}

static int tcp_empty(void) {
  return ( !tcp_queue.full && tcp_queue.front == tcp_queue.back );
}

void tcp_reset(const char *hostname) {
  int rc = close(tcp_socket);
  if ( rc < 0 )
    nl_error(2, "Error closing tcp_socket: %s", strerror(errno));
  tcp_create(hostname);
}

void tcp_connected(void) {
  socklen_t len = sizeof(errno);
  getsockopt(tcp_socket, SOL_SOCKET, SO_ERROR, &errno, &len);
  if (errno == 0) {
    tcp_state = tcp_empty() ? FD_IDLE : FD_WRITE;
    ssp_data.Status = SSP_STATUS_READY;
    nl_error(0, "TCP connected");
  } else {
    nl_error((errno == ETIMEDOUT ? -2 : 2),
      "Error %d during connect, resetting", errno);
    tcp_reset(save_hostname);
  }
}

void tcp_enqueue( char *cmd ) {
  int nb;
  if ( tcp_queue.full )
    nl_error(3, "tcp_queue overflow" );
  nb = snprintf( tcp_queue.q[tcp_queue.back], TCP_SEND_SIZE, "%s\r\n", cmd );
  if ( nb >= TCP_SEND_SIZE )
    nl_error(2, "tcp_enqueue command too long: %s", cmd );
  else {
    if ( ++tcp_queue.back == TCP_QSIZE )
      tcp_queue.back = 0;
    if ( tcp_queue.back == tcp_queue.front )
      tcp_queue.full = 1;
  }
  if ( tcp_state == FD_IDLE ) tcp_state = FD_WRITE;
}

// tcp_send() sends the specified command.
// It returns zero on success and terminates on failure
int tcp_send(void) {
  int rv;
  char *cmd = tcp_queue.q[tcp_queue.front];
  int cmdlen = strlen(cmd);
  nl_error( -3, "tcp_send: '%s'", cmd );
  nl_assert( tcp_state == FD_WRITE );
  rv = send( tcp_socket, cmd, cmdlen, 0);
  if ( rv != cmdlen ) {
    if (errno == EPIPE) {
      nl_error(2, "Saw EPIPE in tcp_send(): resetting TCP");
      tcp_reset(save_hostname);
      return 1;
    } else {
      nl_error(3, "Send failed: %d: errno = %d\n", rv, errno );
    }
  }
  tcp_state = FD_READ;
  return 0;
}

// tcp_recv() reads a line from the tcp_socket. The line should be an ASCII integer, which
// it converts to int and returns.
int tcp_recv() {
  char recv_buf[RECV_BUF_SIZE];
  int rv, i;
  nl_assert( tcp_state == FD_READ );
  rv = recv( tcp_socket, recv_buf, RECV_BUF_SIZE, 0 );
  if ( rv <= 0 ) {
    nl_error(2, "Recv returned %d errno %d cmd='%s'\n", rv, errno,
      tcp_queue.q[tcp_queue.front] );
    rv = -1;
  } else {
    int rnum = 0;
    for ( i = 0; i < rv && isdigit(recv_buf[i]); i++ ) {
      rnum = rnum*10 + recv_buf[i] - '0';
    }
    nl_error( -2, "tcp_send: returning %d, %d from command %s",
	      rv, rnum, tcp_queue.q[tcp_queue.front] );
    rv = rnum;
  }
  tcp_queue.full = 0;
  if ( ++tcp_queue.front == TCP_QSIZE )
    tcp_queue.front = 0;
  tcp_state = tcp_empty() ? FD_IDLE : FD_WRITE;
  return rv;
}

int tcp_close(void) {
  close(tcp_socket);
  return 0;
}
