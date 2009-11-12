/* gpib232.h defines interface to the GPIB232-CTA */
extern int gpib232_cmd( const char *cmd );
extern int gpib232_cmd_read( const char *cmd, char *rep, int nb );
extern int gpib232_wrt( int addr, const char *cmd );
extern int gpib232_wrt_read( int addr, const char *cmd, char *rep, int nb );
extern void gpib232_shutdown( void );
extern int gpib232_init( const char *port );
