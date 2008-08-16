#ifndef MSG_H_INCLUDED
#define MSG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

extern char *msghdr_init(char *default, int argc, char **argv);
extern void msg_init_options(char *hdr, int argc, char **argv);
extern int msg(int level, char *s, ...);

#ifdef __cplusplus
};
#endif

#endif
