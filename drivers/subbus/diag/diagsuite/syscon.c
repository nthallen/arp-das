/*
   syscon.c is the top level diagnostic routine for the system controller
   board.  This could easily grow into a comprehensive diagnostic program
   for the ICC system, but the first step is to verify the data path,
   which means verifying the function of the system controller board.
   Written January 18, 1990
*/

#include <curses.h>
#undef getch
#include <stdio.h>
#ifdef DOS
#include <dos.h>
#endif
#ifdef __QNX__
#include "lat.h"
#define poserr perror
#endif
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <cfg.h>
#include "attribut.h"
#include "syscon.h"
#include "scdiag.h"
#include "define.h"

unsigned char attributes[MAX_ATTRS];

void mvWrtNAttr(WINDOW *mw, int attr, int n, int y, int x) {
  int i;
  char *cp;

  cp = mw->scr_image + y*mw->_bufwidth*2 + x*2 + 1;
  for (i = n; i > 0; i--, cp += 2) *cp = attr;
}

static char logo_str[] =
  "System Controller Diagnostics v2.0, January 7, 1996";

/* the position of these options and qualifiers within these strings
is important, so dont change them without changing set_token().
try and keep all command line stuff to 3 code mnemonics.         */
static char options[]= "aut man con end";
static char qualifiers[]= "log lim";

void set_token(char *,char);
char diagbuf[60]={NULCHR};
char indtst=NULCHR;
short int mode, ender, logend;
char *progname;
FILE *logfd=NULL;    
int loglimit=DEFAULT_LIMIT, logcnt=0;
int status;    
long t;

diagdef diag_list[] = {
#include "diagmenu.h"
};
#define N_DIAGS (sizeof(diag_list)/sizeof(diagdef))

static int diag_number;
#define FIRST_NAME_ROW 7
#define NAME_COL 8
#define NAME_WIDTH 20
#define STATUS_COL (NAME_COL+NAME_WIDTH+2)

void diag_name(char *name) {
  attrset(ATTR_NAME);
  mvaddstr(FIRST_NAME_ROW+diag_number, NAME_COL, name);
}

void diag_status(unsigned char attr, char *text, ...) {
  char buf[100], *offstr, *p1, which[5];
  int i;
  va_list ap;
  long t1;
  
  /* puts diagnostic status string in status attribute to screen,
  logs to logfile if needed and sets the global status variable */
  
  attrset(ATTR_MAIN);
  for (i = 0; i < (COLS - 1 - STATUS_COL); i++) buf[i] = ' ';
  buf[i] = '\0';
  mvaddstr(FIRST_NAME_ROW+diag_number, STATUS_COL, buf);
  va_start(ap, text);
  vsprintf(buf, text, ap);
  va_end(ap);
  attrset(attr);
  mvaddstr(FIRST_NAME_ROW+diag_number, STATUS_COL, buf);
  if (attr==ATTR_FAIL) status=1;
  else if (status==ATTR_WARN&&status<1) status=-1;   
  if (logfd)
     if (logcnt<loglimit) {
        if (attr==ATTR_FAIL || attr==ATTR_WARN) {
           strcpy(which,(attr==ATTR_WARN) ? "warn" : "fail");     
           time(&t1); offstr=ctime(&t1); p1=offstr+11; *(offstr+19)=NULCHR;
           fprintf(logfd,"%s: %s: %s: %s %s\n",progname,p1,which,diag_list[diag_number].name,buf);
           logcnt++;
        }
    }
    else if (!logend) {
        fprintf(logfd,"%s: maximum log limit of %d reached\n",progname,loglimit);
        logend++;
    }
}

void diag_test(int diag_number) {
  wmove(stdscr, FIRST_NAME_ROW+diag_number, STATUS_COL);
  mvWrtNAttr(stdscr, ATTR_EXEC, NAME_WIDTH, FIRST_NAME_ROW+diag_number, NAME_COL);
  refresh();
  diag_list[diag_number].func(mode);
}

int diag_ok(int diag_number) {
  if (indtst!=NULCHR) {
    if (stcpm(diagbuf,diag_list[diag_number].mmm,0)) {
      if (indtst==ENABLE) return(1);
    }
    else if (indtst==DISABLE) return(1);;
  }
  else return(1);  
  return(0);   
}
 
int main(int argc,char *argv[]) {
  WINDOW *logo;
  int width,last_diag, c=0;
  int i;

  /* default conditions */
  mode=AUTO_MODE; ender=0;

  
  /* parse command line */
  progname=argv[0];
  if (strchr(argv[0],DOT))
    strcpy(strchr(progname,DOT),NULSTR);
  for (i=0;i<strlen(progname);i++)
     progname[i]=tolower(progname[i]);
  for (i=1;i<argc;i++)
     set_token(&argv[i][1],argv[i][0]);     

#ifdef syscon
  i = load_subbus();
  if (i==0) {
  printf("No Subbus, Are you sure you want to run %s on THIS node? y/n ",
	   progname);
  i = getchar();
  if (tolower(i)!='y') exit(1);
  }
  
#endif
  
  /* set up the screen */
  init_attrs(CFG_FILE,attributes,MAX_ATTRS);
  if (initscr() == ERR) printf("Cannot initscr()");
  cbreak();
  noecho();
  attrset(ATTR_MAIN);
  clear();
  box(stdscr, VERT_DOUBLE, HORIZ_DOUBLE);
  
  /* set up the logo */
  width = strlen(logo_str)+2;
  logo = subwin(stdscr, 3, width, 2, (COLS-width)/2);
  wattrset(logo, ATTR_LOGO);
  wclear(logo);
  box(logo, VERT_SINGLE, HORIZ_SINGLE);
  mvwaddstr(logo, 1, 1, logo_str);
  delwin(logo);

  /* display the function names */
  for (diag_number = 0; diag_number < N_DIAGS; diag_number++)
    diag_name(diag_list[diag_number].name);
  nodelay(stdscr,TRUE);    
  refresh();

  /* automatic mode and continuous loop */
  if (mode!=MAN_MODE)
    do {
      for (diag_number = 0; diag_number < N_DIAGS; diag_number++) {
        mvWrtNAttr(stdscr, ATTR_NAME, NAME_WIDTH, FIRST_NAME_ROW+diag_number, NAME_COL);  
        if (diag_ok(diag_number)) {
           diag_test(diag_number);
           mvWrtNAttr(stdscr, ATTR_NAME, NAME_WIDTH, FIRST_NAME_ROW+diag_number, NAME_COL);  
           refresh();
         }
      }
      if (wgetch(stdscr)==ESCAPE) break;  
      }    
    while (mode==CON_MODE);        
 
     
  if (mode!=MAN_MODE) indtst=NULCHR;
  mode=MAN_MODE; nodelay(stdscr,FALSE);
 
  
  /* manual mode loop */
  if (!ender) {
    for (diag_number = 0; diag_number < N_DIAGS; diag_number++)
      mvWrtNAttr(stdscr, ATTR_NAME, NAME_WIDTH, FIRST_NAME_ROW+diag_number, NAME_COL);
    diag_number=0;
    while(!diag_ok(diag_number)&&diag_number<N_DIAGS) diag_number++;  
    mvWrtNAttr(stdscr, ATTR_HILT, NAME_WIDTH, FIRST_NAME_ROW+diag_number, NAME_COL);
    keypad(stdscr,TRUE);
    refresh();
    while(1) {
      last_diag=diag_number;
      switch (c=wgetch(stdscr)) {
    	case KEY_UP:
          while (!diag_ok(diag_number=(diag_number==0)?N_DIAGS-1:diag_number-1));
          break;
    	case KEY_DOWN:
          while(!diag_ok(diag_number=(diag_number==N_DIAGS-1)?0:diag_number+1));
          break;
        case CR:
          if (diag_ok(diag_number)) diag_test(diag_number);
          break;  
        default: break;
       }
       if (c==ESCAPE) break;
       if (last_diag != diag_number)
         mvWrtNAttr(stdscr, ATTR_NAME, NAME_WIDTH, FIRST_NAME_ROW+last_diag, NAME_COL);
       mvWrtNAttr(stdscr, ATTR_HILT, NAME_WIDTH, FIRST_NAME_ROW+diag_number, NAME_COL);
       refresh();
    } 
  }

 /* clean up */
 refresh();
 attrset(7);
 clear();
 if (logfd) {
   time(&t); fprintf(logfd,"program %s ended at %s",progname,ctime(&t));
 }
 refresh();
 endwin();
 return(status);
}

void usage(void) {
int i; char buf[30], in[3]={NULCHR};
printf("%s [?] [/QUAL=<id>]... [+|-OPTION]...\n   where QUAL is in {%s}\n   and OPTION is in {%s}\n   or \
any of the three code mnemonics of the following tests\n",
progname,qualifiers,options);
  for (i=0;i<N_DIAGS;i++)
      printf("         %s %s\n",diag_list[i].name,diag_list[i].mmm);
  puts("continue with more instructions? y/n"); gets(in);
  if (in[0]=='y' || in[0]=='Y') {
     sprintf(buf,"more %s",HELP_FILE);
     if (system(buf)==-1)
        poserr(progname);
  }
  exit(0);    
}

/* command line switch settings, store enabled/disabled tests */
void set_token(char *token, char able) {
  char *mtchptr, *p, hold[6];
  int index;
  switch (able) {
    case QUALIFIER: p=strchr(token,EQUAL);
                    index=p-token;
                    strncpy(hold,token,index);
                    hold[index]=NULCHR; p++;
                    if (stcpm(qualifiers,hold,&mtchptr))
                      switch(mtchptr-qualifiers) {
                        case 0: if (!(logfd=fopene(p,"a",0))) {
                                   puts(p); poserr(progname); exit(0);
                                }
                                time(&t);
                                fprintf(logfd,"program %s started at %s",progname,ctime(&t));
                                break;
                        case 4: loglimit=atoi(p); break;
                      }
                    break;
    case USAGE: usage(); break;
    case ENABLE: case DISABLE:    
         if (stcpm(options,token,&mtchptr))
            switch (mtchptr-options) {
               case 0: switch(able) {
                          case DISABLE: mode=MAN_MODE; break;
                          default: break;
                       }
                       break;
               case 4: switch(able) {
                          case ENABLE: mode=MAN_MODE; break;
                          default: break;
                       }
                       break;
               case 8: switch(able) {
                          case ENABLE: mode=CON_MODE; break;
                          default: break;
                       }
                       break;
               case 12: switch(able) {
                           case ENABLE: ender=1; break;
                           default: break;
                        }
                        break;
             }
         else {
            if (indtst==NULCHR) indtst=able;
            if (indtst==able) { 
            strcat(diagbuf,token);
            strcat(diagbuf,BLANKSTR);
          }
       }
    }
}


