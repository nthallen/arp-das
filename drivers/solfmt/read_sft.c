/* read_sft.c reads in a .sft file as per the printed specification.
   Written April 9, 1987
   Upgraded to Lattice V6.0 April 13, 1990
   Modified July 1991 for QNX.
   Ported to QNX 4 by Eil 4/20/92.
*/
#include <stdio.h>
#include <stdlib.h>
#include "soldrv.h"
#include "codes.h"
#include "version.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int n_solenoids, n_set_points, n_modes, n_bytes, *mode_indices;
solenoid *solenoids;
set_point *set_points;
unsigned char *mode_code;

int fget_word(FILE *fp) {
  int w;

  w = fgetc(fp) & 0xFF;
  w += fgetc(fp) << 8;
  return(w);
}

/* zero on sucess, 1 equals cant open, -1 = wrong version */
int read_sft(char *filename) {
  FILE *fp;
  int i;

  fp = fopen(filename, "rb");
  if (!fp) return(1);
  if (fget_word(fp) != VERSION) return(-1);
  n_solenoids = fget_word(fp);
  solenoids = (solenoid *)malloc(n_solenoids*sizeof(solenoid));
  for (i = 0; i < n_solenoids; i++) {
    solenoids[i].open_cmd = fget_word(fp);
    solenoids[i].close_cmd = fget_word(fp);
    solenoids[i].status_addr = fget_word(fp);
    solenoids[i].status_mask = fget_word(fp);
  }
  n_set_points = fget_word(fp);
  set_points = (set_point *)malloc(n_set_points*sizeof(set_point));
  for (i = 0; i < n_set_points; i++) {
    set_points[i].address = fget_word(fp);
    set_points[i].value = fget_word(fp);
  }
  n_modes = fget_word(fp);
  mode_indices = (int *)malloc(n_modes * sizeof(int));
  for (i = 0; i < n_modes; i++) mode_indices[i] = fget_word(fp);
  n_bytes = fget_word(fp);
  mode_code = (unsigned char *)malloc(n_bytes);
  for (i = 0; i < n_bytes; i++) mode_code[i] = fgetc(fp);
  fclose(fp);
}
