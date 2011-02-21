/*      READ_D2A.C
        Handles the DtoA definitions from the solenoid cycle definition
        file.
        Written April 23, 1990.
        Modified July 1991 for QNX.
*/
#include <string.h>
#include <ctype.h>
#include "tokens.h"
#include "dtoa.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

set_point set_points[MAX_SET_POINTS];
int n_set_points = 0;

void read_dtoa(void) {
  int token, i;
  unsigned int address;

  if (n_dtoas == MAX_DTOAS) filerr("Too Many DtoAs Defined\n");
  token = get_token();
  if (token == TK_DTOA_NAME) filerr("Attempted Redefinition of DtoA\n");
  if (token != TK_UNDEFINED_STRING) filerr("DtoA Name Error\n");
  dtoas[n_dtoas].name = strdup(gt_input);
  if (get_token() != TK_NUMBER) filerr("DtoA Addresss Missing\n");
  address = gt_number;
  if (get_token() != TK_LBRACE) filerr("DtoA Set Point Definition needs '{'\n");
  for (i = 0; i < MAX_DTOA_SET_POINTS; i++) {
    for (token = gt_getc(); isspace(token); token = gt_getc()) ;
    if (token == '}') {
      gt_ungetc(token);
      break;
    } else dtoas[n_dtoas].set_point_name[i] = token;
    if (get_token() != TK_COLON) filerr("DtoA Set Point Definition needs ':'\n");
    if (get_token() != TK_NUMBER) filerr("DtoA Set Point Value Missing\n");
    set_points[n_set_points].address = address;
    set_points[n_set_points].value = gt_number;
    dtoas[n_dtoas].set_point_index[i] = n_set_points;
    n_set_points++;
  }
  dtoas[n_dtoas].n_set_points = i;
  if (get_token() != TK_RBRACE) filerr("DtoA Set Point Definition needs '}'\n");
  n_dtoas++;
}
