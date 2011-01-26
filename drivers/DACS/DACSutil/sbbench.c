#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "nortlib.h"
#include "subbus.h"

#define TEST_ADDR 0x0A00
#define FIRST_GUESS 100

int time_loop( int n ) {
  struct timespec start, end;
  int i, rv, diff;
  unsigned short data;

  clock_gettime(CLOCK_REALTIME, &start);
  for ( i = 0; i < n; i++ ) {
    rv = read_ack( TEST_ADDR, &data );
  }
  clock_gettime(CLOCK_REALTIME, &end);
  diff = (end.tv_sec - start.tv_sec)*1000000 +
    (end.tv_nsec - start.tv_nsec)/1000;
  return diff;
}

int main(int argc, char **argv) {
  clock_t dt = 0;
  int n;

  if ( load_subbus() == 0 )
    nl_error( 3, "Unable to load subbus library" );

  for (n = FIRST_GUESS; dt == 0; ) {
    dt = time_loop( n );
    if ( dt == 0 ) n *= 4;
  }
  printf("First guess: %d took %d clocks\n", n, dt );
  if ( dt < CLOCKS_PER_SEC/2 ) {
    n *= CLOCKS_PER_SEC/dt;
    printf("Trying %d\n", n);
    dt = time_loop( n );
  }
  printf("Final: %d took %d clocks: %.0lf/sec, %.0lf usec/read\n",
    n, dt, ((double)n)*CLOCKS_PER_SEC/dt, 1e6*dt/(n*CLOCKS_PER_SEC) );
  if (argc > 1) {
    printf("Looping...\n");
    for (;;) {
      dt = time_loop(n);
      printf("Final: %d took %d clocks: %.0lf/sec, %.0lf usec/read\n",
	n, dt, ((double)n)*CLOCKS_PER_SEC/dt, 1e6*dt/(n*CLOCKS_PER_SEC) );
    }
  }
  return 0;
}
