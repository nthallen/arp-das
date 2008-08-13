#include <sys/types.h>
#include <unistd.h>
#include <process.h>
#include <assert.h>

int main(int argc, char **argv) {
  seteuid(getuid());
  execvp(argv[1], argv+1);
  return 1;
}

#ifdef __USAGE
%C	<program> [args ...]
	Resets the effective UID to the real UID.
#endif
