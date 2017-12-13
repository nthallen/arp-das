/* calibr.h defines structures and functions pertinent to calibration. */
#ifndef _TMCSTR_H
  #error Must include tmcstr.h before calibr.h
#endif

#ifndef _CALIBR_H
#define _CALIBR_H
struct pair {
  struct pair *next;
  double v[2];
};
struct pairlist {
  struct pair *pairs;
  unsigned int npts;
};
struct calibration {
  struct calibration *next;
  struct nm *type[2];
  unsigned int flag;
  struct pairlist pl;
};
#define CALB_XUNIQ 1
#define CALB_YMONO 2
#define CALB_YINC 4
#define CALB_YDEC 8

void add_pair(struct pairlist *list, double v0, double v1); /* calibr.c */
void add_calibration(struct nm *type0, struct nm *type1, struct pairlist *pl); /* calibr.c */
struct cvtfunc *specify_conv( struct tmtype *ftype,
        struct cvtfunc *cfn, int cfntype ); /* calibr.c */
void classify_conv( struct tmtype *ftype );
#endif
