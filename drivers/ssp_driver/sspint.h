/* sspint.h: Internal defintions for sspdrv */
enum fdstate ( FD_IDLE, FD_READ, RD_WRITE );
extern enum fdstate tcp_state, udp_state;

extern int tcp_create( int board_id );
extern void tcp_enqueue( char *cmd );
extern int tcp_send(void);
extern int tcp_recv(void);
extern int udp_create(void);
extern int udp_receive(long int *scan, size_t length );
extern void udp_close(void);
