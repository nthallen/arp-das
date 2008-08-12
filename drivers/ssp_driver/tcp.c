#include <ctype.h>
#include "nl_assert.h"
#include "sspint.h"

static int tcp_socket;
enum fdstate tcp_state = FD_IDLE;

#define TCP_QSIZE 20
#define TCP_SEND_SIZE 30
static struct {
  int front, back, full;
  char q[TCP_QSIZE][TCP_SEND_SIZE];
} tcp_queue;

/**
 * returns socket fd or dies
 */
int tcp_create( int board_id ) {

  int rc;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;
  char hostname[40];

  nl_assert( board_id >= 0 && board_id < 55 );
  snprintf( hostname, 40, "10.0.0.%d", 200 + board_id );
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
    nl_error( 3, "Cannot bind TCP port %u: %s", SSP_SERVER_PORT, strerror(errno));
        
  /* connect to server */
  rc = connect(tcp_socket, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if (rc<0)
    nl_error( 3, "Cannot connect: %s", strerror(errno) );
  tcp_queue.front = tcp_queue.back = 0;
  tcp_queue.full = 0;
  return tcp_socket;
}

static int tcp_empty(void) {
  return ( !tcp_queue.full && tcp_queue.front == tcp_queue.back );
}

void tcp_enqueue( char *cmd ) {
  if ( tcp_queue.full )
    nl_error(3, "tcp_queue overflow" );
  strncpy( tcp_queue.q[tcp_queue.back], cmd, TCP_SEND_SIZE );
  if ( tcp_queue.q[tcp_queue.back][TCP_SEND_SIZE-1] != '\0' )
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
  nl_assert( tcp_state == FD_WRITE );
  rv = send( tcp_socket, cmd, cmdlen, 0);
  if ( rv != cmdlen )
    nl_error(3, "Send failed: %d: errno = %d\n", rv, errno );
  tcp_state = FD_READ;
  return 0;
}

// tcp_recv() reads a line from the tcp_socket. The line should be an ASCII integer, which
// it converts to int and returns.
int tcp_recv() {
  char recv_buf[RECV_BUF_SIZE];
  int rv, i;
  nl_assert( tcp_state == FD_READ );
  tcp_state = tcp_empty() ? FD_IDLE : FD_WRITE;
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
  return rv;
}

int tcp_close(void) {
  close(tcp_socket);
  return 0;
}
