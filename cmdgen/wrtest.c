#include <sys/console.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void main() {
  struct _console_ctrl *cc;
  int fd;
  int row, col, type;
  char *buf;
  
  fd = open("/dev/con1", O_RDWR);
  cc = console_open(fd, O_RDWR);
  close(fd);
  
  buf = calloc(80*25, 2);
  console_read(cc, 0, 0, buf, 80*25*2, &row, &col, &type);
  console_write(cc, 0, 2*(row*80 + col),
		"H\007e\007l\007l\007o\007!\007", 2*6, NULL, NULL, NULL);
  console_write(cc, 2, 2*(row*80 + col),
		"H\007e\007l\007l\007o\007!\007", 2*6, NULL, NULL, NULL);
  console_write(cc, 3, 2*(row*80 + col),
		"H\007e\007l\007l\007o\007!\007", 2*6, NULL, NULL, NULL);
  getch();
  console_write(cc, 0, 0, buf, 80*25*2, &row, &col, &type);
  console_close(cc);
}
