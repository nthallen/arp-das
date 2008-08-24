#include <curses.h>
#undef getch
#ifdef DOS
#include <dos.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "define.h"
#include <stdlib.h>
#include "subbus.h"
#include "attribut.h"
#include <ctype.h>
#include <cfg.h>

char *to_str(char *out, int dat, int radix, int res) {
/* returns a string of 'dat' in specified radix and resolution */    

#define HIGH(x) (((x) >> 8) & 0xFF)
#define LOW(x) ((x)&255)

    int and,j;
    unsigned int mask;
    switch (radix) {
        case HEX: switch (res) {
                     case BYTERES: sprintf(out,"%02X %02X",HIGH(dat),LOW(dat));
                                   break;
                     case WORDRES: sprintf(out,"%04X",dat);
                }
                break;
        case BIN: out[0]=NULCHR; j=0;
                for (mask=(unsigned)(1 << (WORDSIZE-1));mask!=0;mask >>= 1) {
                    if (res==BYTERES && j==(WORDSIZE-BYTESIZE))
                       out[j++]=BLANK;
                    and=dat&mask;
                    out[j++]=(and) ? '1':'0';
                }
                out[j]=NULCHR;
                break;
        case DEC: switch (res) {
                     case BYTERES: sprintf(out,"%3d %3d",HIGH(dat),LOW(dat));
                                   break;
                     case WORDRES: sprintf(out,"%6d",dat);
                                   break;
                }
                break;
    }
    return(out);
}


int disp_addrs(int from, int to, int radix, int res, int cmds[], int numcmds, int addrs[], int numcycaddr) {
    
#define BOXES { wattrset(dispwin,ATTR_EXEC); wattrset(statwin,ATTR_EXEC); \
                box(dispwin,VERT_DOUBLE,HORIZ_DOUBLE); \
                box(statwin,VERT_SINGLE,HORIZ_SINGLE); }
#define REFRESHER { pnoutrefresh(statwin,0,0,1,2,6+(int)lines,11+(int)cols); \
                    pnoutrefresh(dispwin,0,0,3,6,4+(int)lines,7+(int)cols); \
                    doupdate(); }
#define CLRADDR { wmove(dispwin,(int)lines,1); \
                  for (k=0;k<FIELD_ADDR;k++) \
                  waddch(dispwin,BLANK); BOXES;}
#define CLRDATA(x,y) { wmove(dispwin,(x),(y)); \
                       for(k=0;k<FIELD_DATA;k++) waddch(dispwin,BLANK); \
                       BOXES; }
#define CLRSTAT(x,c) { wattrset(statwin,ATTR_EXEC); \
                       wmove(statwin,statline,(x)); \
                       if ((c)==B||(c)==b) k=0; \
                       else if ((c)==M||(c)==m) k=4; else k=10; \
                       if (k!=10) for(kk=0;kk<k;kk++) waddch(statwin,BLANK); \
                       else wclrtoeol(statwin); }
#define STATUS(a,c) { a1=ATTR_PASS; if ((c)==M||(c)==m) statcol=1; \
                      else if ((c)==B||(c)==b) statcol=5; \
                      else { statcol=10; a1=(a); } \
                      CLRSTAT(statcol,c); wattrset(statwin,a1); \
                      mvwaddstr(statwin,statline,statcol,statbuf); BOXES; }
#define CHGADDR { CLRADDR; \
                  mvwaddstr(dispwin,(int)lines,1,to_str(out,from,HEX,WORDRES)); \
                  mvwaddch(dispwin,(int)lines,1+FIELD_ADDR,COLON); }
#define CHGDATA { CLRDATA((int)lines,FIELD_ADDR+2); \
                  mvwaddstr(dispwin,(int)lines,FIELD_ADDR+2,to_str(out,dat,radix,res)); }       
#define GETRAD { switch (radix) { \
                     case HEX: strcpy(statbuf,HEXSTAT); break; \
                     case DEC: strcpy(statbuf,DECSTAT); break; \
                     case BIN: strcpy(statbuf,BINSTAT); break; } }
#define GETRES { switch (res) { \
                     case BYTERES: strcpy(statbuf,BSTAT); break; \
                     case WORDRES: strcpy(statbuf,WDSTAT); break; } }
#define SETUP_REC(str,x,y,z) nodelay(dispwin,FALSE); \
                   strcpy(statbuf,str); STATUS(att,c); \
                   if (z==0) CLRADDR else CLRDATA((int)lines,FIELD_ADDR+2); \
                   wmove(dispwin,x,y); \
                   REFRESHER; echo(); wgetstr(dispwin,out);
#define UNSET_REC(z) statbuf[0]=NULCHR; \
                   if (z==0) CHGADDR else CHGDATA; \
                   STATUS(att,c); REFRESHER; \
                   noecho(); nodelay(dispwin,TRUE);\
                   break;
    
    int c, dat=0,  i, j, k, kk, numcols, ok=0, cycle=0, step=1;
    float lines,cols,numaddr;
    WINDOW *dispwin, *statwin;
    char out[19], statbuf[30];
    unsigned char att, a1;
    int stat,proceed, statline, statcol=0, addrsindex=0;
    unsigned int maxlim=0xFFFF, minlim=0;
    long int chk;
    
    
    /* calculate size of display pad from number of addresses */
    
    from = (from%2) ? from+1 :from;
    numaddr=ceil((abs(to-from)+1)/2.0);
    lines=(numaddr>MAXLINES) ? MAXLINES : numaddr;
    numcols=ceil(numaddr/MAXLINES);
    cols=numcols*(FIELD_ADDR+1+FIELD_DATA);
    if (cols>MAXCOLS) return(0);
    statline=(int)lines+4;
    stat=1; att=ATTR_PASS;
    
    dispwin=newpad((int)lines+2,(int)cols+2);
    statwin=newpad((int)lines+6,(int)cols+10);
    wclear(dispwin); wclear(statwin);
    keypad(dispwin,TRUE); BOXES;
    
    /* display commands, filter out upper case letters */
    wattrset(statwin,ATTR_HILT);
    for (i=0,j=1;i<numcmds;i++)
         switch(cmds[i]) {
            case ESCAPE:mvwaddstr(statwin,1,j,ESCSTR); j+=sizeof(ESCSTR)-1; break;
            case CTRLR:mvwaddstr(statwin,1,j,CTRLRSTR); j+=sizeof(CTRLRSTR)-1; break;
            case CTRLW:mvwaddstr(statwin,1,j,CTRLWSTR); j+=sizeof(CTRLWSTR)-1; break;
            case CTRLL:mvwaddstr(statwin,1,j,CTRLLSTR); j+=sizeof(CTRLLSTR)-1; break;
            case CTRLC:mvwaddstr(statwin,1,j,CTRLCSTR); j+=sizeof(CTRLCSTR)-1; break;
            case CR:mvwaddstr(statwin,1,j,CRSTR); j+=sizeof(CRSTR)-1; break;
            case KEY_UP:mvwaddstr(statwin,1,j,KEY_UPSTR); j+=sizeof(KEY_UPSTR)-1; break;
            case KEY_DOWN:mvwaddstr(statwin,1,j,KEY_DOWNSTR); j+=sizeof(KEY_DOWNSTR)-1; break;
            case KEY_LEFT:mvwaddstr(statwin,1,j,KEY_LSTR); j+=sizeof(KEY_LSTR)-1; break;
            case KEY_RIGHT:mvwaddstr(statwin,1,j,KEY_RSTR); j+=sizeof(KEY_RSTR)-1; break;
            default:if (!isupper(cmds[i])) {
                 mvwaddch(statwin,1,j,(char)cmds[i]); j++;

            }
        }
    
    /* display status */
    GETRES; STATUS(att,B);
    GETRAD; STATUS(att,M);
    if (numaddr>1) {
       strcpy(statbuf,CRSTAT);
       STATUS(att,CTRLR);
    }
    
    
    /* display address(es) */
    statbuf[0]=NULCHR;
    for (i=0;i<numcols;i++)
       for (j=0;j<lines;j++) {
          mvwaddstr(dispwin,j+1,i*COL_WIDTH+1,to_str(out,from+((i*MAXLINES)+j)*2,HEX,WORDRES));
          mvwaddch(dispwin,j+1,i*COL_WIDTH+1+FIELD_ADDR,COLON);
          if (i*j>=numaddr) break;
       }
    REFRESHER;   
    nodelay(dispwin,TRUE);
    
    
    /* handle user commands, first check if it is an enabled command */
    while ((c=wgetch(dispwin))!=ESCAPE) {
       proceed=0; i=0;
       if (c!=ERR)
          while (!proceed && i<numcmds)
             if (c==cmds[i]) proceed++; else i++;
       if ((ok>2||cycle) &&proceed && (c!=CR&&c!=M&&c!=m&&c!=B&&c!=b))proceed=0;
       if (!proceed&&cycle) { c=CTRLC; proceed++; }
       if (!proceed&&ok==99) { c=CTRLW; proceed++; }
       if (!proceed&&ok==88) { c=CTRLR; proceed++; }
       if (proceed)   
       switch (c) {
           case M: case m: radix++; radix%=3; CHGDATA; GETRAD; break;
           case B: case b: res++; res%=2; CHGDATA; GETRES; break;
           case R: case r: ok=1; strcpy(statbuf,RSTAT); break;
           case W: case w: ok=1; strcpy(statbuf,WSTAT); break;
           case S: case s:
                   SETUP_REC(STEPSTAT,1,FIELD_ADDR+2,1);
                   stch_i(out,&step); step=(step) ? step : 1;
                   UNSET_REC(1);
           case A: case a:
                   SETUP_REC(ADDRSTAT,1,1,0);
                   stch_i(out,&from);
                   from=(from%2) ? from+1 : from; CHGADDR;
                   UNSET_REC(0);
            case D: case d:
                   SETUP_REC(DATASTAT,1,FIELD_ADDR+2,1);
                   stch_i(out,&dat);
                   if (dat>maxlim) dat=minlim;
                   if (dat<minlim) dat=maxlim;
                   UNSET_REC(1);
            case L: case l:
                   SETUP_REC(MAXLIMSTAT,1,FIELD_ADDR+2,1);
                   stch_i(out,&maxlim); maxlim=(maxlim) ? maxlim : 0xFFFF;
                   UNSET_REC(1);
            case CTRLL:       
                   SETUP_REC(MINLIMSTAT,1,FIELD_ADDR+2,1);
                   stch_i(out,&minlim); minlim=(minlim) ? minlim : 0;
                   UNSET_REC(1);
            case PLUS:
                  if (numcycaddr) {
                     if (++addrsindex>(numcycaddr-1)) addrsindex=0;
                     from=addrs[addrsindex];
                     from=(from%2) ? from+1 : from;
                     strcpy(statbuf,ACUSTAT); }
                  else {
                     from+=2; strcpy(statbuf,AISTAT); }
                   CHGADDR; break;
            case MINUS:
                 if (numcycaddr) {
                    if (--addrsindex<0) addrsindex=numcycaddr;
                    from=addrs[addrsindex];
                    from=(from%2) ? from+1 : from;
                    strcpy(statbuf,ACDSTAT); }
                 else {            
                    from-=2; strcpy(statbuf,ADSTAT); }
                    CHGADDR; break;
            case CTRLC: cycle=1;
            case KEY_RIGHT: c=W; ok=1;
            case KEY_UP: 
                   chk=dat; chk+=step; dat+=step;
                   if (chk>maxlim) dat=minlim;
                   if (dat<minlim) dat=maxlim;            
                   if (cycle) strcpy(statbuf,CCSTAT);
                   else if (c==W) strcpy(statbuf,CUSTAT);
                   else strcpy(statbuf,DISTAT);
                   CHGDATA; break;
            case KEY_LEFT: c=W; ok=1;
            case KEY_DOWN: dat-=step;
                   if (dat>maxlim) dat=minlim;
                   if (dat<minlim) dat=maxlim;            
                   if (c==W) strcpy(statbuf,CDSTAT);
                   else strcpy(statbuf,DDSTAT);
                   CHGDATA; break;
            case CR: strcpy(statbuf,NOP); ok=0; cycle=0; break;     
            case CTRLR: ok=88; strcpy(statbuf,CRSTAT); break;
            case CTRLW: ok=99; strcpy(statbuf,CWSTAT); break;
            default: c=ERR;
       }
       else  c=ERR;
       
       /* read/write from/to address(es) */
       if (numaddr>1||c==CTRLW||c==CTRLR||c==CTRLC||ok==1) {
          /* get and display again */
          for (i=0;i<numcols;i++)
              for (j=0;j<lines;j++) {
                  /* read the from+(i*MAXLINES)+j address */
                  /* put output at j lines, i cols+FIELD_ADDR+1 */
                  CLRDATA(j+1,i*COL_WIDTH+FIELD_ADDR+2);
                  if (c==CTRLW || c==W || c==w)
                     stat=write_ack(0,from,dat);
                  else   
                     stat=read_ack(0,from+((i*MAXLINES)+j)*2,(unsigned far *)&dat);
                  mvwaddstr(dispwin,j+1,i*COL_WIDTH+FIELD_ADDR+2,to_str(out,dat,radix,res));
                  if (i*j>=numaddr) break;
               }
               if (c==W || c==R || c==w || c==r) { c=CR; ok=0; }
        }
        
        /* status display */
        if (!stat) att=ATTR_FAIL;
        if (c!=ERR) STATUS(att,c);
        REFRESHER;  
        stat=1; att=ATTR_PASS;
    }
    
    /* clean up */
    delwin(dispwin);  delwin(statwin);
    wrefresh(stdscr);  return(1);
}



