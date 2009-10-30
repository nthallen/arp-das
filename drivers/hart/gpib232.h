/* gpib232.h defines interface to the GPIB232-CTA */
int gpib232_read_data( char *buf, int count, int addr );
double gpib232_read_double( int addr );
short int gpib232_read_status( void );
void gpib232_command( int lvl, char *msg );
int gpib232_init( const char *port );
/* optional: invoked via atexit() after gpib232_init() */
void gpib232_shutdown( void );
