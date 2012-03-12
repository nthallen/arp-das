#include <errno.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <time.h>
#include "SerSelector.h"
#include "nortlib.h"
#include "oui.h"
#include "tmdf.h"

const char *df_path = "/";
TMDF_t TMDF;

class TMDF_Selectee : public TM_Selectee {
  public:
    TMDF_Selectee( int seconds, const char *name, void *data,
	  unsigned short size );
    ~TMDF_Selectee();
    int Process_Data(int flag);
  private:
    int fd;
    int secs;
    time_t next;
};

TMDF_Selectee::TMDF_Selectee( int seconds, const char *name,
	void *data, unsigned short size )
    : TM_Selectee(name, data, size ) {
  fd = open(df_path, O_RDONLY);
  next = 0;
  if (fd < 0) {
    nl_error( 2, "Error opening %s: %s", df_path,
      strerror(errno) );
  }
}

TMDF_Selectee::~TMDF_Selectee() {
  if (fd >= 0) close(fd);
}

int TMDF_Selectee::Process_Data(int flag) {
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
      }
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  oui_init_options(argc, argv);
  { Selector S;
    Cmd_Selectee QC;
    TMDF_Selectee TM( 60, "TMDF", &TMDF, sizeof(TMDF));
    S.add_child(&QC);
    S.event_loop();
  }
  nl_error(0, "Terminating");
}

