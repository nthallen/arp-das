#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "nortlib.h"
#include "nctable.h"
#include "nl_assert.h"

#define MAX_DEVS 34

typedef struct {
  char *dev_name;
  SCREEN *screen;
  FILE *ofp, *ifp;
  int ifd;
  int dirty;
} nct_display_t;

static nct_display_t nct_display[MAX_DEVS];
static int n_devs = 0;
static int n_scrs = 0;
static int cur_scr_num;
static char *ttype;
static int ifds_opened = 0;

static inline void nct_select(int n) {
  nl_assert(n < n_scrs );
  if (cur_scr_num != n) {
    cur_scr_num = n;
    set_term(nct_display[n].screen);
  }
}

static inline void mark_dirty(void) {
  nct_display[cur_scr_num].dirty = 1;
}

static inline int is_dirty(int n) {
  return nct_display[n].dirty;
}

void nct_args( char *dev_name ) {
  if ( n_devs >= MAX_DEVS )
    nl_error( 2, "Too many devices specified" );
  else nct_display[n_devs++].dev_name = dev_name;
}

static void nct_shutdown(void) {
  int i;
  for ( i = 0; i < n_scrs; i++ ) {
    nct_display_t *d = &nct_display[i];
    set_term(d->screen);
    clear();
    refresh();
    reset_prog_mode();
    endwin();
    delscreen(d->screen);
    fclose(d->ofp);
    fclose(d->ifp);
  }
  while ( ifds_opened > 0 ) {
    close(nct_display[--ifds_opened].ifd);
  }
}

int nct_init( const char *winname, int n_rows, int n_cols ) {
  FILE *ofp, *ifp;
  char *dev_name;
  SCREEN *scr;
  nct_display_t *d;

  if ( n_scrs >= n_devs )
    nl_error( 3, "Not enough devices specified" );
  if ( ttype == NULL ) {
    ttype = getenv("TERM");
    if ( ttype == NULL )
      nl_error(3, "Cannot determine terminal type");
  }
  d = &nct_display[n_scrs];
  dev_name = d->dev_name;
  d->ofp = ofp = fopen( dev_name, "w" );
  if (ofp == NULL)
    nl_error( 3, "Cannot open %s for write", dev_name ); 
  d->ifp = ifp = fopen( dev_name, "r" );
  if (ifp == NULL)
    nl_error( 3, "Cannot open %s for read", dev_name );
  d->screen = scr = newterm(ttype, ofp, ifp );
  d->dirty = 0;
  set_term(scr);
  def_prog_mode();
  curs_set(0);
  if ( n_scrs == 0 )
    atexit( &nct_shutdown );
  return n_scrs++;
}

void nct_refresh(void) {
  int i;
  for (i = 0; i < n_scrs; i++) {
    if (is_dirty(i)) {
      nct_select(i);
      refresh();
      nct_display[i].dirty = 0;
    }
  }
}

/** \brief Write string to an ncurses display
 *
 * winnum is the window number returned by nct_init();
 * attr is the atribute number (a small integer) as set
 * by nctable. This should map to screen colors, but
 * the implementation of that will have to wait.
 *
 */
void nct_string( int winnum, int attr, int row, int col, const char *text ) {
  nct_select(winnum);
  // color_set(color_pair_table(attr), NULL);
  mvaddstr(row, col, text);
  mark_dirty();
}

void nct_clear( int winnum ) {
  nct_select(winnum);
  clear();
  mark_dirty();
}

static unsigned char grphchar[81] = {
  '\x20', /*  0 = 0000 */
  '\xB3', /*  1 = 0001 */
  '\xBA', /*  2 = 0002 */
  '\xB3', /*  3 = 0010 */
  '\xB3', /*  4 = 0011 */
  '\xBA', /*  5 = 0012 */
  '\xBA', /*  6 = 0020 */
  '\xBA', /*  7 = 0021 */
  '\xBA', /*  8 = 0022 */
  '\xC4', /*  9 = 0100 */
  '\xDA', /* 10 = 0101 */
  '\xD6', /* 11 = 0102 */
  '\xC0', /* 12 = 0110 */
  '\xC3', /* 13 = 0111 */
  '\xD6', /* 14 = 0112 */
  '\xD3', /* 15 = 0120 */
  '\xD3', /* 16 = 0121 */
  '\xC7', /* 17 = 0122 */
  '\xCD', /* 18 = 0200 */
  '\xD5', /* 19 = 0201 */
  '\xC9', /* 20 = 0202 */
  '\xD4', /* 21 = 0210 */
  '\xC6', /* 22 = 0211 */
  '\xC9', /* 23 = 0212 */
  '\xC8', /* 24 = 0220 */
  '\xC8', /* 25 = 0221 */
  '\xCC', /* 26 = 0222 */
  '\xC4', /* 27 = 1000 */
  '\xBF', /* 28 = 1001 */
  '\xB7', /* 29 = 1002 */
  '\xD9', /* 30 = 1010 */
  '\xB4', /* 31 = 1011 */
  '\xB7', /* 32 = 1012 */
  '\xBD', /* 33 = 1020 */
  '\xBD', /* 34 = 1021 */
  '\xB6', /* 35 = 1022 */
  '\xC4', /* 36 = 1100 */
  '\xC2', /* 37 = 1101 */
  '\xD2', /* 38 = 1102 */
  '\xC1', /* 39 = 1110 */
  '\xC5', /* 40 = 1111 */
  '\xD2', /* 41 = 1112 */
  '\xD0', /* 42 = 1120 */
  '\xD0', /* 43 = 1121 */
  '\xD7', /* 44 = 1122 */
  '\xCD', /* 45 = 1200 */
  '\x20', /* 46 = 1201 */
  '\x20', /* 47 = 1202 */
  '\x20', /* 48 = 1210 */
  '\x20', /* 49 = 1211 */
  '\x20', /* 50 = 1212 */
  '\x20', /* 51 = 1220 */
  '\x20', /* 52 = 1221 */
  '\x20', /* 53 = 1222 */
  '\xCD', /* 54 = 2000 */
  '\xB8', /* 55 = 2001 */
  '\xBB', /* 56 = 2002 */
  '\xBE', /* 57 = 2010 */
  '\xB5', /* 58 = 2011 */
  '\x20', /* 59 = 2012 */
  '\xBC', /* 60 = 2020 */
  '\x20', /* 61 = 2021 */
  '\xB9', /* 62 = 2022 */
  '\x20', /* 63 = 2100 */
  '\x20', /* 64 = 2101 */
  '\x20', /* 65 = 2102 */
  '\x20', /* 66 = 2110 */
  '\x20', /* 67 = 2111 */
  '\x20', /* 68 = 2112 */
  '\x20', /* 69 = 2120 */
  '\x20', /* 70 = 2121 */
  '\x20', /* 71 = 2122 */
  '\xCD', /* 72 = 2200 */
  '\xD1', /* 73 = 2201 */
  '\xCB', /* 74 = 2202 */
  '\xCF', /* 75 = 2210 */
  '\xD8', /* 76 = 2211 */
  '\x20', /* 77 = 2212 */
  '\xCA', /* 78 = 2220 */
  '\x20', /* 79 = 2221 */
  '\xCE'  /* 80 = 2222 */
};

static unsigned char asciichar[81] = {
  ' ', /*  0 = LRTB */
  ',', /*  1 = 0001 */
  ',', /*  2 = 0002 */
  '\'', /*  3 = 0010 */
  '|', /*  4 = 0011 */
  '|', /*  5 = 0012 */
  '"', /*  6 = 0020 */
  '|', /*  7 = 0021 */
  '|', /*  8 = 0022 */
  '-', /*  9 = 0100 */
  '+', /* 10 = 0101 */
  '+', /* 11 = 0102 */
  '+', /* 12 = 0110 */
  '|', /* 13 = 0111 */
  '|', /* 14 = 0112 */
  '+', /* 15 = 0120 */
  '+', /* 16 = 0121 */
  '+', /* 17 = 0122 */
  '=', /* 18 = 0200 */
  '+', /* 19 = 0201 */
  '+', /* 20 = 0202 */
  '+', /* 21 = 0210 */
  '+', /* 22 = 0211 */
  '+', /* 23 = 0212 */
  '+', /* 24 = 0220 */
  '+', /* 25 = 0221 */
  '+', /* 26 = 0222 */
  '-', /* 27 = 1000 */
  '+', /* 28 = 1001 */
  '+', /* 29 = 1002 */
  '+', /* 30 = 1010 */
  '+', /* 31 = 1011 */
  '+', /* 32 = 1012 */
  '+', /* 33 = 1020 */
  '+', /* 34 = 1021 */
  '+', /* 35 = 1022 */
  '-', /* 36 = 1100 */
  '+', /* 37 = 1101 */
  '+', /* 38 = 1102 */
  '+', /* 39 = 1110 */
  '+', /* 40 = 1111 */
  '+', /* 41 = 1112 */
  '+', /* 42 = 1120 */
  '+', /* 43 = 1121 */
  '+', /* 44 = 1122 */
  '-', /* 45 = 1200 */
  '+', /* 46 = 1201 */
  '+', /* 47 = 1202 */
  '+', /* 48 = 1210 */
  '+', /* 49 = 1211 */
  '+', /* 50 = 1212 */
  '+', /* 51 = 1220 */
  '+', /* 52 = 1221 */
  '+', /* 53 = 1222 */
  '=', /* 54 = 2000 */
  '+', /* 55 = 2001 */
  '+', /* 56 = 2002 */
  '+', /* 57 = 2010 */
  '+', /* 58 = 2011 */
  '+', /* 59 = 2012 */
  '+', /* 60 = 2020 */
  '+', /* 61 = 2021 */
  '+', /* 62 = 2022 */
  '-', /* 63 = 2100 */
  '+', /* 64 = 2101 */
  '+', /* 65 = 2102 */
  '+', /* 66 = 2110 */
  '+', /* 67 = 2111 */
  '+', /* 68 = 2112 */
  '+', /* 69 = 2120 */
  '+', /* 70 = 2121 */
  '+', /* 71 = 2122 */
  '=', /* 72 = 2200 */
  '+', /* 73 = 2201 */
  '+', /* 74 = 2202 */
  '+', /* 75 = 2210 */
  '+', /* 76 = 2211 */
  '+', /* 77 = 2212 */
  '+', /* 78 = 2220 */
  '+', /* 79 = 2221 */
  '+'  /* 80 = 2222 */
};

static unsigned char *nct_boxchars = grphchar;

void nct_charset(int n) {
  switch (n) {
    case NCT_CHARS_GR:
      nct_boxchars = grphchar;
      break;
    case NCT_CHARS_ASCII:
      nct_boxchars = asciichar;
      break;
    default:
      nl_error(2, "Invalid charset code" );
  }
}

void nct_hrule( int winnum, int attr, int row, int col,
		unsigned char *rule ) {
  unsigned char *r = rule;
  nct_select(winnum);
  // color_set(color_pair_table(attr), NULL);
  while (*r) {
    mvaddch(row, col, nct_boxchars[*r]);
    ++r;
    ++col;
  }
}

void nct_vrule( int winnum, int attr, int row, int col,
		unsigned char *rule ) {
  unsigned char *r = rule;
  nct_select(winnum);
  // color_set(color_pair_table(attr), NULL);
  while (*r) {
    mvaddch(row, col, nct_boxchars[*r]);
    ++r;
    ++row;
  }
}

#include <fcntl.h>
#include <unix.h>
#include <sys/select.h>

#define IBUFSIZE 16

char nct_getch(void) {
  static char ibuf[IBUFSIZE];
  static int nc = 0;
  static int bp = 0;

  while ( ifds_opened < n_devs ) {
    nct_display_t *d = &nct_display[ifds_opened++];
    d->ifd = open( d->dev_name, O_RDONLY|O_NONBLOCK );
    if ( d->ifd == -1 )
      nl_error(3, "Unable to read from %s: %d", d->dev_name, errno );
    curs_set(1); // Turn on the cursor
  }
  for (;;) {
    if ( nc > 0 && bp < nc ) {
      return ibuf[bp++];
    } else {
      int i, width = 0, rv;

      nc = 0;
      bp = 0;
      fd_set fs;
      FD_ZERO(&fs);
      for ( i = 0; i < ifds_opened; i++ ) {
	int ifd = nct_display[i].ifd;
	FD_SET( ifd, &fs );
	if ( ifd >= width )
	  width = ifd + 1;
      }
      rv = select(width, &fs, NULL, NULL, NULL);
      if ( rv == -1 ) {
	if ( errno != EINTR )
	  nl_error( 3, "Error %d from select", errno );
      } else {
	for ( i = 0; i < n_devs; i++ ) {
	  int ifd = nct_display[i].ifd;
	  if ( FD_ISSET( ifd, &fs ) ) {
	    int nb = read( ifd, &ibuf[nc], IBUFSIZE-nc );
	    if ( nb == -1 ) {
	      if (errno != EAGAIN && errno != EINTR)
		nl_error( 3, "Error %d from read()", errno );
	    } else if ( nb == 0 )
	      nl_error( 1, "Read 0 bytes" );
	    else {
	      nc += nb;
	    }
	  }
	}
      }
    }
  }
}
