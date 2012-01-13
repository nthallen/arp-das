#ifndef QCLIPROG_H_INCLUDED
#define QCLIPROG_H_INCLUDED
extern int opt_w;
extern int opt_v;
void qcliprog_init( int argc, char **argv );
int verify_block( unsigned short addr, unsigned short *prog, int blocklen );
#endif
