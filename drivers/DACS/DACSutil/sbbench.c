#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "nortlib.h"
#include "nl_assert.h"
#include "subbus.h"

#define TEST_ADDR 0x0C00
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

int time_loop2( subbus_mread_req *mrq, int n ) {
  struct timespec start, end;
  int i, rv, diff, last_err;
  unsigned short data[16];
  int saw_problem = 0;


  clock_gettime(CLOCK_REALTIME, &start);
  for ( i = 0; i < n; i++ ) {
    rv = mread_subbus( mrq, &data[0] );
    if (rv != 0) {
      saw_problem = 1;
      last_err = rv;
    }
  }
  clock_gettime(CLOCK_REALTIME, &end);
  diff = (end.tv_sec - start.tv_sec)*1000000 +
    (end.tv_nsec - start.tv_nsec)/1000;
  if ( saw_problem )
    printf("  An error (%d) was reported during mread_subbus\n", last_err);
  return diff;
}

int main(int argc, char **argv) {
  clock_t dt = 0;
  int n;
  subbus_mread_req *mrq;

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
  printf("read_ack: %d took %d clocks: %.0lf/sec, %.0lf usec/read\n",
    n, dt, ((double)n)*CLOCKS_PER_SEC/dt, 1e6*dt/(n*CLOCKS_PER_SEC) );

  printf("\nMulti-read 16 words per call\n");
  mrq = pack_mread_request( 16, "C00:2:C1E" );
  nl_assert(mrq != NULL);
  printf( "mrq->req_len = %d\n", mrq->req_len );
  printf( "mrq->n_reads = %d\n", mrq->n_reads );
  printf( "mrq->multread_cmd = %s", mrq->multread_cmd );

  for (n = FIRST_GUESS; dt == 0; ) {
    dt = time_loop2( mrq, n );
    if ( dt == 0 ) n *= 4;
  }
  printf("First guess: %d took %d clocks\n", n, dt );
  if ( dt < CLOCKS_PER_SEC/2 ) {
    n *= CLOCKS_PER_SEC/dt;
    printf("Trying %d\n", n);
    dt = time_loop2( mrq, n );
  }
  printf("mread_subbus: %d took %d clocks: %.0lf/sec, %.0lf usec/read\n",
    n, dt, ((double)n)*16*CLOCKS_PER_SEC/dt, 1e6*dt/(16*n*CLOCKS_PER_SEC) );
  if (argc > 1) {
    printf("Looping...\n");
    for (;;) {
      dt = time_loop(n);
      printf("read_ack: %d took %d clocks: %.0lf/sec, %.0lf usec/read\n",
	n, dt, ((double)n)*CLOCKS_PER_SEC/dt, 1e6*dt/(n*CLOCKS_PER_SEC) );

      dt = time_loop2( mrq, n);
      printf("mread_subbus: %d took %d clocks: %.0lf/sec, %.0lf usec/read\n",
	n, dt, ((double)n)*16*CLOCKS_PER_SEC/dt, 1e6*dt/(16*n*CLOCKS_PER_SEC) );
    }
  }
  return 0;
}
