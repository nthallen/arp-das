#ifndef CMDCLT_H_INCLUDED
#define CMDCLT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* Header "nlphcmd.h" for cltwindow Application */
extern int nlph_getch(void);
extern void nlph_cmdclt_init( void *(*cmd_thread)(void*));
extern void nlph_update_cmdtext( char *cmdtext, char *prompttext );

#ifdef __cplusplus
};
#endif

#endif
