/* proxies.h defines structures used for the Proxy statement.
 * $Log$
 * Revision 1.4  2006/02/16 18:13:24  nort
 * Uncommitted changes
 *
 * Revision 1.3  2002/05/10 16:10:42  nort
 * Increased some arbitrary limits
 *
 * Revision 1.2  1993/09/28  17:07:56  nort
 * *** empty log message ***
 *
 * Revision 1.1  1992/09/21  18:21:44  nort
 * Initial revision
 *
 */
#define MAX_PROXY_POINTS 40
#define MAX_PROXIES 256
#define MAX_PRXY_PTS 100

typedef struct {
  char *name;
  int n_proxies;
  int proxy_name[MAX_PROXY_POINTS];
  int proxy_index[MAX_PROXY_POINTS];
  int last_time;
  int last_state;
  int first_state;
} proxy;

extern int n_proxies, n_prxy_pts;
extern proxy proxies[MAX_PROXIES];
extern int prxy_pts[MAX_PRXY_PTS];
