/* oui.h provides the prototype for oui_init_options()
 * $Log$
 * Revision 1.2  1995/05/26 14:44:45  nort
 * Added opt_string definition!
 *
 * Revision 1.1  1994/11/22  14:54:36  nort
 * Initial revision
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

void oui_init_options(int argc, char **argv);
extern char *opt_string;

#ifdef __cplusplus
};
#endif
