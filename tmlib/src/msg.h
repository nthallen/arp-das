#ifndef MSG_H_INCLUDED
#define MSG_H_INCLUDED

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const char *msghdr_init(const char *hdr_default, int argc, char **argv);
extern void msg_init_options(const char *hdr, int argc, char **argv);
extern int msg(int level, const char *s, ...);
extern int msgv(int level, const char *s, va_list args);
extern void msg_set_hdr(const char *hdr);

#ifdef __cplusplus
};
#endif

#endif
