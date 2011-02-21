/* tokens.h again.
   $Log$
   Revision 1.2  1993/09/28 17:08:10  nort
   *** empty log message ***

 * Revision 1.1  1992/09/21  18:21:44  nort
 * Initial revision
 *
   Written March 23, 1987
*/
#define TK_SOLENOID 0
#define TK_OPEN 1
#define TK_CLOSE 2
#define TK_RESOLUTION 3
#define TK_MODE 4
#define TK_EQUAL 5
#define TK_COLON 6
#define TK_LBRACE 7
#define TK_RBRACE 8
#define TK_SLASH 9
#define TK_EOF 10
#define TK_NUMBER 11
#define TK_SOLENOID_NAME 12
#define TK_INITIALIZE 13
#define TK_CHAR_CONSTANT 14
#define TK_UNDEFINED_STRING 15
#define TK_ROUTINE 16
#define TK_ROUTINE_NAME 17
#define TK_SELECT 18
#define TK_STATUS_BYTES 19
#define TK_DTOA 20
#define TK_DTOA_NAME 21
#define TK_PROXY 22
#define TK_PROXY_NAME 23
#define TK_CMD_SET 24

extern char gt_input[];
extern int gt_number;
void filerr(char * cntrl,...);
int get_token(void), gt_spos(long int pos, int line);
int open_token_file(char *name);
int gt_getc(void);
void gt_ungetc(int c);
long int gt_fpos(int *line);
