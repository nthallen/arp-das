/* rdtest.c
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/dev.h>
#include <sys/stat.h>
#include <sys/kernel.h>
#include <assert.h>
#include <fcntl.h>

#define IBUFSZ 40
#define MAXCONS 4
#ifndef CMD_ERROR
  #define CMD_ERROR(x) do { fprintf(stderr, "%s", x); exit(1); } while (0)
#endif

struct condef {
  int fd;
  unsigned old_mode;
  pid_t proxy;
} cons[MAXCONS];
unsigned int n_cons = 0;

/* Returns zero on success. Else errno is set. */
int define_con(char *name) {
  struct condef *con;
  
  if (n_cons < MAXCONS) {
    con = &cons[n_cons++];
	con->fd = open(name, O_RDWR);
	if (con->fd >= 0) {
	  con->old_mode = dev_mode(con->fd, 0, _DEV_ECHO | _DEV_EDIT | _DEV_ISIG);
	  con->proxy = qnx_proxy_attach(0, NULL, 0, -1);
	  if (con->proxy != -1) {
		if (dev_arm(con->fd, con->proxy, _DEV_EVENT_INPUT) != -1)
		  return(0);
	  }
	}
  }
  return(1);
}

void close_cons(void) {
  struct condef *con;
  
  while (n_cons > 0) {
    con = &cons[--n_cons];
	qnx_proxy_detach(con->proxy);
	dev_mode(con->fd, con->old_mode, _DEV_MODES);
	close(con->fd);
  }
}

int con_getch(void) {
  pid_t who;
  int i;
  static unsigned char ibuf[IBUFSZ], *bptr;
  static unsigned int nchars = 0;

  if (n_cons == 0) CMD_ERROR("No consoles open");
  while (nchars == 0) {
	who = Receive(0, NULL, 0);
	for (i = 0; i < n_cons; i++) {
	  if (who == cons[i].proxy) {
		/* input from cons[i] */
		nchars = read(cons[i].fd, ibuf, IBUFSZ);
		assert(nchars > 0);
		bptr = ibuf;
		if (dev_arm(cons[i].fd, cons[i].proxy, _DEV_EVENT_INPUT) == -1)
		  CMD_ERROR("Error arming console");
		break;
	  }
	}
	#ifdef RECVFUNC
	  if (i == n_cons) RECVFUNC(who);
	#endif
  }
  assert(nchars > 0);
  nchars--;
  return(*bptr++);
}

int main(int argc, char **argv) {
  int i;
  
  for (i = 1; i < argc; i++) {
	if (define_con(argv[i]))
	  fprintf(stderr, "Error opening console %s\n", argv[i]);
  }
  do {
	i = con_getch();
	printf("%02X\n", i);
  } while (i != '\033');
  close_cons();
  return(0);
}
