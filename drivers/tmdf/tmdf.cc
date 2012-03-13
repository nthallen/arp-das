#include <errno.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <time.h>
#include "SerSelector.h"
#include "nortlib.h"
#include "oui.h"
#include "tmdf.h"
#include "tmdf_int.h"

const char *df_path = "/";
const char *tmdf_name = "TMDF";
TMDF_t TMDF;

class TMDF_Selectee : public TM_Selectee {
  public:
    TMDF_Selectee( unsigned seconds, const char *name, void *data,
	  unsigned short size );
    ~TMDF_Selectee();
    int ProcessData(int flag);
  private:
    int fd;
    unsigned secs;
    time_t next;
};

TMDF_Selectee::TMDF_Selectee( unsigned seconds, const char *name,
	void *data, unsigned short size )
    : TM_Selectee(name, data, size ) {
  fd = open(df_path, O_RDONLY);
  next = 0;
  secs = seconds;
  if (fd < 0) {
    nl_error( 2, "Error opening %s: %s", df_path,
      strerror(errno) );
  }
}

TMDF_Selectee::~TMDF_Selectee() {
  if (fd >= 0) close(fd);
}

int TMDF_Selectee::ProcessData(int flag) {
  if (fd >= 0) {
    time_t now = time(NULL);
    if ( next == 0 || now >= next ) {
      struct statvfs buf;
      next = now + secs;
      if (fstatvfs(fd, &buf) ) {
	nl_error(2, "fstatvfs reported %s", strerror(errno));
      } else {
	double blks;
	blks = (buf.f_blocks - buf.f_bavail);
	blks = blks * 65535. / buf.f_blocks;
	TMDF.usage = (blks > 65535) ? 65535 :
	  ((unsigned short)blks);
	nl_error(-2, "f_blocks = %d  f_bavail = %d",
	  buf.f_blocks, buf.f_bavail );
      }
    } else {
      nl_error(-3, "next: %lu  now: %lu", next, now );
    }
  }
  return TM_Selectee::ProcessData(flag);
}

int main(int argc, char **argv) {
  oui_init_options(argc, argv);
  nl_error(0, "Startup");
  { Selector S;
    Cmd_Selectee QC;
    TMDF_Selectee TM( 60, tmdf_name, &TMDF, sizeof(TMDF));
    S.add_child(&QC);
    S.add_child(&TM);
    S.event_loop();
  }
  nl_error(0, "Terminating");
}

