/* read_cmd.c handles the top level input from a solenoid format file.
   $Log$
   Revision 1.4  2006/02/16 18:13:25  nort
   Uncommitted changes

 * Revision 1.3  1993/09/28  17:14:41  nort
 * *** empty log message ***
 *
 * Revision 1.2  1993/09/28  17:07:03  nort
 * *** empty log message ***
 *
 * Revision 1.1  1992/09/21  18:21:44  nort
 * Initial revision
 *
   Written March 24, 1987
   Modified July 1991 for QNX.
*/
#include "tokens.h"
#include "solfmt.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int cmd_set = 'A';

void read_cmd(void) {
  int token;
  extern int open_char, close_char, res_num, res_den;

  for (token = get_token(); token != TK_EOF; token = get_token()) {
    switch (token) {
	  case TK_CMD_SET:
        if (get_token() != TK_EQUAL || get_token() != TK_CHAR_CONSTANT)
          filerr("Command_Set Statement Requires '='\n");
		if (gt_number < 'A' || gt_number > 'J')
		  filerr("Command_Set Value Out of Range");
		cmd_set = gt_number;
        break;
      case TK_SOLENOID:
        read_sol();
        break;
      case TK_OPEN:
      case TK_CLOSE:
        if (get_token() != TK_EQUAL || get_token() != TK_CHAR_CONSTANT)
          filerr("Open/Close Statement Syntax Error\n");
        if (token == TK_OPEN) open_char = gt_number;
        else close_char = gt_number;
        break;
      case TK_RESOLUTION:
        if (get_token() != TK_EQUAL) filerr("Expected '=' after 'resolution'");
        if (get_token() != TK_NUMBER)
          filerr("Expected <number> after 'resolution ='");
        res_num = gt_number;
        if (get_token() != TK_SLASH) filerr("Need Slash in resolution");
        if (get_token() != TK_NUMBER) filerr("Need Number after slash");
        res_den = gt_number;
        break;
      case TK_MODE:
        read_mode();
        break;
      case TK_ROUTINE:
        read_routine();
        break;
      case TK_STATUS_BYTES:
        read_status_addr();
        break;
      case TK_DTOA:
        read_dtoa();
        break;
	  case TK_PROXY:
		read_proxy();
		break;
      default: filerr("Syntax Error in read_cmd\n");
    }
  }
}
