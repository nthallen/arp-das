/* read_sol.c handles the solenoid definition statements from the solenoid
   format file.
   $Log$
   Revision 1.3  2006/02/16 18:13:28  nort
   Uncommitted changes

 * Revision 1.2  1993/09/28  17:14:55  nort
 * *** empty log message ***
 *
 * Revision 1.1  1992/09/21  18:21:44  nort
 * Initial revision
 *
   Written March 24, 1987
   Modified July 1991 for QNX.
*/
#include <string.h>
#include "tokens.h"
#include "solenoid.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

void read_sol(void) {
  int token;

  if (n_solenoids == MAX_SOLENOIDS) filerr("Too Many Solenoids Defined\n");
  token = get_token();
  if (token == TK_SOLENOID_NAME) filerr("Attempted Redefinition of Solenoid\n");
  if (token != TK_UNDEFINED_STRING) filerr("Solenoid Name Error\n");
  solenoids[n_solenoids].name = strdup(gt_input);
  if (get_token() != TK_NUMBER) filerr("Solenoid Open Command Missing\n");
  solenoids[n_solenoids].open_cmd = gt_number;
  if (get_token() != TK_NUMBER) filerr("Solenoid Close Command Missing\n");
  solenoids[n_solenoids].close_cmd = gt_number;
  if (get_token() != TK_NUMBER)
    filerr("Solenoid Status Bit Definition Missing\n");
  if (gt_number >= 32 || gt_number < 0)
    filerr("Solenoid Status Bit Out of Range\n");
  solenoids[n_solenoids].status_bit = gt_number;
  n_solenoids++;
}
