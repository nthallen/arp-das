/* output.c handles writing the ".sft" file.
 * $Log$
 * Revision 1.3  2011/02/23 19:38:40  ntallen
 * Changes for DCCC
 *
 * Revision 1.2  2011/02/22 18:40:37  ntallen
 * Solfmt compiled
 *
 * Revision 1.1  2011/02/21 18:26:05  ntallen
 * QNX4 version
 *
 * Revision 1.3  1993/09/28 17:17:11  nort
 * *** empty log message ***
 *
 * Revision 1.2  1993/09/28  17:06:59  nort
 * *** empty log message ***
 *
   Written April 8, 1987
   Modified July 1991 for QNX.
*/
#include <stdio.h>
#include <string.h>
#include "tokens.h"
#include "solenoid.h"
#include "modes.h"
#include "dtoa.h"
#include "proxies.h"
#include "version.h"
#include "solfmt.h"
#include "nortlib.h"

static char rcsid[] =
      "$Id$";

#define MAX_STAT_ADDR        4
static int status_addrs[MAX_STAT_ADDR] = { 0x408, 0x40A, 0x411, 0x413 };

void fput_word(int w, FILE *fp) {
  fputc(w & 0xFF, fp);
  fputc((w >> 8) & 0xFF, fp);
}

void output(char *ofile) {
  FILE *fp;
  int i, max_mode;

  fp = fopen(ofile, "wb");
  if (fp == NULL) nl_error(3,"Cannot open output file \"%s\"\n", ofile);
  fput_word(VERSION, fp);
  fputc(cmd_set, fp);

  /* Output solenoid definitions */
  fput_word(n_solenoids, fp);
  for (i = 0; i < n_solenoids; i++) {
    fput_word(solenoids[i].open_cmd, fp);
    fput_word(solenoids[i].close_cmd, fp);
    fput_word(status_addrs[solenoids[i].status_bit/8], fp);
    if (solenoids[i].status_bit >= 16)
      fput_word(1 << (8 + (solenoids[i].status_bit % 8)), fp);
    else fput_word(1 << (solenoids[i].status_bit % 8), fp);
  }

  /* Output set point definitions */
  fput_word(n_set_points, fp);
  for (i = 0; i < n_set_points; i++) {
    fput_word(set_points[i].address, fp);
    fput_word(set_points[i].value, fp);
  }

  /* Output proxy definitions */
  fput_word(n_prxy_pts, fp);
  for (i = 0; i < n_prxy_pts; i++)
	fputc(prxy_pts[i], fp);

  for (i = 0, max_mode = 0; i < MAX_MODES;)
    if (modes[i++].index != -1) max_mode = i;
  fput_word(max_mode, fp);
  for (i = 0; i < max_mode; i++) fput_word(modes[i].index, fp);
  fput_word(mci, fp);
  for (i = 0; i < mci; i++) fputc(mode_code[i], fp);
  
  /* Now output the string table. First the strings, preceeded by
     a count of the total size */
  { int n_bytes = 0;
    for ( i = 0; i < n_dccc_strs; i++ ) {
      n_bytes += strlen(dccc_strs[i]) + 1; // for the NUL
    }
    fput_word(n_bytes, fp);
    for ( i = 0; i < n_dccc_strs; i++ ) {
      char *s = dccc_strs[i];
      do {
        fputc( *s, fp );
      } while (*s++ != '\0');
    }
    /* Now output number of strings and a table of offsets */
    fput_word( n_dccc_strs, fp );
    for ( n_bytes = 0, i = 0; i < n_dccc_strs; i++ ) {
      fput_word( n_bytes, fp );
      n_bytes += strlen(dccc_strs[i]) + 1; // for the NUL
    }
  }
  fclose(fp);
  printf("Output written to file %s\n", ofile);
}

void read_status_addr(void) {
  int token, i = 0;

  if (get_token() != TK_LBRACE) filerr("Expected '{' after 'status_bytes'");
  for (token = get_token(); token == TK_NUMBER; i++, token = get_token()) {
    if (i == MAX_STAT_ADDR)  filerr("Maximum of 4 status addresses");
    status_addrs[i] = gt_number;
  }
  if (token != TK_RBRACE) filerr("Expected '}' after status addresses");
}
