/* Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>
#include <Pt.h>
#include "nortlib.h"
#include "tablelib.h"
#include "nlphcmd.h"


static sem_t char_sem;
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
#define QUEUE_LENGTH 10
typedef struct {
  int head;
  int tail;
  int chars[QUEUE_LENGTH];
} queue_t;
static queue_t queue;
static int my_key_event( PtWidget_t *widget, void *apinfo,
          PtCallbackInfo_t *cbinfo );

static PtWidget_t *cmd_window;
static PtWidget_t *cmd_text;
static PtWidget_t *cmd_prompt;

static PtWidget_t *new_cmd_window( char *title, int width, int height ) {
  PtArg_t args[8];
  PtSetArg(&args[0], Pt_ARG_WINDOW_TITLE, title, 0);
  PtSetArg(&args[1], Pt_ARG_WINDOW_MANAGED_FLAGS,
      Pt_FALSE, Ph_WM_MAX | Ph_WM_RESIZE );
  PtSetArg(&args[2], Pt_ARG_WINDOW_MANAGED_FLAGS,
      Pt_TRUE, Ph_WM_COLLAPSE );
  PtSetArg(&args[3], Pt_ARG_WIDTH, width, 0);
  PtSetArg(&args[4], Pt_ARG_HEIGHT, height, 0);
  PtSetArg(&args[5], Pt_ARG_WINDOW_RENDER_FLAGS,
      Pt_FALSE, Ph_WM_RENDER_MAX | Ph_WM_RENDER_RESIZE );
  PtSetArg(&args[6], Pt_ARG_WINDOW_RENDER_FLAGS,
      Pt_TRUE, Ph_WM_RENDER_COLLAPSE );
  PtSetArg(&args[7], Pt_ARG_FILL_COLOR, PgGray(100), 0);
  return PtCreateWidget( PtWindow, Pt_NO_PARENT, 8, args );
}

static PtWidget_t *new_text_widget( PtWidget_t *window, char *text,
                int x, int y, char *font, PgColor_t color, int align ) {
  PhPoint_t pos;
  PtArg_t args[12];

  pos.x = x; pos.y = y;
  PtSetArg(&args[0], Pt_ARG_TEXT_STRING, text, 0);
  PtSetArg(&args[1], Pt_ARG_POS, &pos, 0 );
  PtSetArg(&args[2], Pt_ARG_TEXT_FONT, font, 0 );
  PtSetArg(&args[3], Pt_ARG_COLOR, color, 0 );
  PtSetArg(&args[4], Pt_ARG_HORIZONTAL_ALIGNMENT, align, 0 );
  PtSetArg(&args[5], Pt_ARG_MARGIN_WIDTH, 4, 0 );
  PtSetArg(&args[6], Pt_ARG_MARGIN_HEIGHT, 4, 0 );
  PtSetArg(&args[7], Pt_ARG_FILL_COLOR, PgGray(220), 0);
  PtSetArg(&args[8], Pt_ARG_BASIC_FLAGS,
    Pt_HORIZONTAL_GRADIENT | Pt_REVERSE_GRADIENT |
    Pt_TOP_OUTLINE | Pt_LEFT_OUTLINE,
    Pt_HORIZONTAL_GRADIENT | Pt_REVERSE_GRADIENT |
    Pt_TOP_OUTLINE | Pt_LEFT_OUTLINE | Pt_FLAT_FILL |
    Pt_ALL_BEVELS );
  PtSetArg(&args[9], Pt_ARG_OUTLINE_COLOR, PgGray(20), 0);
  PtSetArg(&args[10], Pt_ARG_FLAGS, Pt_TRUE, Pt_HIGHLIGHTED );
  // PtSetArg(&args[11], Pt_ARG_BEVEL_WIDTH, 2, 0 );
  return PtCreateWidget(PtLabel, window, 11, args);
}

#define LABEL_HEIGHT 25
#define LABEL_FULL_HEIGHT (LABEL_HEIGHT+4)
#define LABEL_SPACE 6
#define LABEL_WIDTH 780
#define LABEL_FULL_WIDTH (LABEL_WIDTH+4)
#define WINDOW_HEIGHT (2*LABEL_FULL_HEIGHT + 3*LABEL_SPACE + 1)
#define WINDOW_WIDTH (LABEL_FULL_WIDTH  + 2*LABEL_SPACE)
#define LOGO_WIDTH 20

void nlph_cmdclt_init( void *(*cmd_thread)(void*)) {
  pthread_attr_t attr;
  PhDim_t dim;

  PtInit(NULL);
  cmd_window = new_cmd_window("cmd_name", WINDOW_WIDTH, WINDOW_HEIGHT );
  cmd_prompt = new_text_widget( cmd_window, "prompt",
	LABEL_SPACE, LABEL_FULL_HEIGHT + 2*LABEL_SPACE,
         "FixedFont10", Pg_BLACK, Pt_LEFT );
  dim.w = LABEL_WIDTH; dim.h = LABEL_HEIGHT;
  PtSetResource( cmd_prompt, Pt_ARG_DIM, &dim, 0 );

  cmd_text = new_text_widget( cmd_window, "text",
        LOGO_WIDTH + 2*LABEL_SPACE, LABEL_SPACE,
         "FixedFont10", Pg_BLACK, Pt_LEFT );
  dim.w = LABEL_WIDTH-LOGO_WIDTH-LABEL_SPACE;
  dim.h = LABEL_HEIGHT;
  PtSetResource( cmd_text, Pt_ARG_DIM, &dim, 0 );

  // PgDrawPolygon();
  // Wrong: need to create a PtPolygon widget.
  PtAddFilterCallback( cmd_window, Ph_EV_KEY, my_key_event, NULL );

  if ( sem_init( &char_sem, 0, 0 ) )
    nl_error( 4, "sem_init failed" );

  pthread_attr_init( &attr );
  //pthread_attr_setdetachstate(
  //    &attr, PTHREAD_CREATE_DETACHED );
  pthread_create( NULL, &attr, cmd_thread, NULL );

  PtRealizeWidget(cmd_window);
  PtMainLoop();
}

static void enqueue_char( int c ) {
  int rv;
  rv = pthread_mutex_lock( &queue_mutex );
  if ( rv == EOK ) {
    int newtail = queue.tail + 1;
    if (newtail >= QUEUE_LENGTH) newtail = 0;
    if (newtail != queue.head ) {
      queue.chars[queue.tail] = c;
      queue.tail = newtail;
      rv = sem_post( &char_sem );
      if ( rv != 0 )
	nl_error( 2, "sem_post returned errno %d", errno );
    }
    rv = pthread_mutex_unlock( &queue_mutex );
    if ( rv != EOK )
      nl_error( 2, "pthread_mutex_unlock returned %d", rv );
  } else nl_error( 2, "pthread_mutex_lock returned %d", rv );
}

int nlph_getch( void ) {
  int rv, c;
  rv = sem_wait(&char_sem);
  if ( rv != 0 ) nl_error( 4, "sem_wait returned errno %d", errno );
  rv = pthread_mutex_lock( &queue_mutex );
  if ( rv == EOK ) {
    if ( queue.head == queue.tail )
      nl_error( 4, "char queue is empty" );
    c = queue.chars[queue.head];
    if ( ++queue.head >= QUEUE_LENGTH )
      queue.head = 0;
    rv = pthread_mutex_unlock( &queue_mutex );
    if ( rv != EOK )
      nl_error( 2, "pthread_mutex_unlock returned %d", rv );
    return c;
  } else nl_error( 4, "pthread_mutex_lock returned %d", rv );
  return 0;
}

static int my_key_event( PtWidget_t *widget, void *apinfo,
          PtCallbackInfo_t *cbinfo ) {
  PhEvent_t *event = cbinfo->event;
  PhKeyEvent_t *keyevent = PhGetData(event);

  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;

  if ( keyevent->key_flags & Pk_KF_Sym_Valid ) {
    unsigned long sym = keyevent->key_sym;
    /* we will only enqueue characters cmdgen understands */
    switch ( sym ) {
      case 0xF008: sym = 0x8; break;
      case 0xF009: sym = 0x9; break;
      case 0xF00D: sym = 0xD; break;
      case 0xF01B: sym = 0x1B; break;
      case 0xF07F: sym = 0x7F; break;
      default:
	if ( sym >= 0x80 || !isprint(sym) )
	  sym = 0;
	break;
    }
    if (sym) {
      // if ( isgraph(sym) ) nl_error( 0, "enqueue: '%c'", sym );
      // else nl_error( 0, "enqueue: 0x%X", sym );
      enqueue_char(sym);
    }
  } else {
    nl_error(-3, "  %s%s%s %s %s%lX\n",
      (keyevent->key_mods & Pk_KM_Ctrl) ? "C" : " ",
      (keyevent->key_mods & Pk_KM_Shift) ? "S" : " ",
      (keyevent->key_mods & Pk_KM_Alt) ? "A" : " ",
      (keyevent->key_flags & Pk_KF_Key_Down) ? "v" : "^",
      (keyevent->key_flags & Pk_KF_Cap_Valid) ? "OK:" : "X:",
      keyevent->key_cap );
  }
  return( Pt_CONTINUE );
}

static char *cur_prompt_text;
void nlph_update_cmdtext( char *cmdtext, char *prompttext ) {
  PtEnter(0);
  if ( prompttext != cur_prompt_text ) {
    cur_prompt_text = prompttext;
    PtSetResource( cmd_prompt, Pt_ARG_TEXT_STRING, cur_prompt_text, 0 );
  }
  PtSetResource( cmd_text, Pt_ARG_TEXT_STRING, cmdtext, 0 );
  PtLeave(0);
}
