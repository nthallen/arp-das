/* cmdalgo.h defines entry points into the cmdgen-based algorithms
 * Part of the cmdgen distribution
 */
#ifndef CMDALGO_H_INCLUDED
#define CMDALGO_H_INCLUDED

#ifdef __cplusplus

#include "collect.h"

extern "C" {
#endif

void cmd_init(void);
void cmd_interact(void);
int cmd_batch(char *cmd, int test);
typedef struct {
  unsigned short state;
  unsigned short value;
} cmd_state;
void cmd_report(cmd_state *s);
int cmd_check(cmd_state *s);
void cis_initialize(void); /* in cmdgen.skel or .cmd */
void cis_terminate(void);  /* in cmdgen.skel of .cmd */
void cis_interfaces(void); /* generated */
void cis_interfaces_close(void); /* generated */
#define CMDREP_QUIT 1000
#define CMDREP_SYNERR 2000
#define CMDREP_EXECERR 3000
#define CMDREP_TYPE(x) ((x)/1000)

struct ioattr_s;
#define IOFUNC_ATTR_T struct ioattr_s
extern IOFUNC_ATTR_T *cis_setup_rdr( char *node );
extern void cis_turf( IOFUNC_ATTR_T *handle, char *format, ... );
void ci_server(void); /* in tmlib/cis.c */
#define CMDSRVR_NAME "cmd/server"
#define CMD_INTERP_MAX 256
#define CMD_VERSION_MAX 80

#ifdef __cplusplus
};

class cmdif_rd {
  public:
    cmdif_rd( const char *name );
    void Setup();
    void Turf(const char *fmt, ...);
    void Shutdown();
  private:
    const char *name;
    IOFUNC_ATTR_T *handle;
};

class cmdif_wr {
  public:
    cmdif_wr(const char *name, const char *path);
    void Setup();
    void Turf(const char *fmt, ...);
    void Shutdown();
  private:
    const char *name;
    const char *path;
    int ofd;
};

class cmdif_dgdata {
  public:
    cmdif_dgdata(const char *name, void *data, int dsize);
    // void Setup(); // does nothing.
    void Turf();
    void Shutdown();
  private:
    const char *name;
    void *data;
    int dsize;
    send_id id;
};

#endif

#endif
