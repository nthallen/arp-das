#ifndef CMDCLT_H_INCLUDED
#define CMDCLT_H_INCLUDED

/* Header "nlphcmd.h" for cltwindow Application */
extern int nlph_getch(void);
extern void nlph_cmdclt_init( void *(*cmd_thread)(void*));
extern void nlph_update_cmdtext( char *cmdtext, char *prompttext );
#endif
