/* postproc.c Handles output processing after all the crucial code has
   been generated.
   $Log$
   Revision 1.2  2008/07/03 18:18:48  ntallen
   To compile under QNX6 with minor blind adaptations to changes between
   dbr.h and tm.h

   Revision 1.1  2008/07/03 15:11:07  ntallen
   Copied from QNX4 version V1R9

   Revision 1.12  2004/10/08 17:34:31  nort
   Read the VERSION file for tm.dac

   Revision 1.11  2001/03/14 15:27:42  nort
   Added #define _Address generation

   Revision 1.10  1999/06/24 17:51:44  nort
   Change limit for 32-bit compile

 * Revision 1.9  1993/09/27  19:40:05  nort
 * Cleanup.
 *
 * Revision 1.8  1993/04/01  22:05:04  nort
 * Restructuring
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include "nortlib.h"
#include "rational.h"
#include "tmcstr.h"
#include "tmc.h"
#include "tm.h" /* defines tm_info_type */

static char rcsid[] =
      "$Id$";

static void print_mfcopy(void) {
  struct nm *nameref;
  struct tmalloc *tmd;
  struct cw *cwl;
  
  nameref = find_name(MFC_NAME, 0);
  assert(name_test(nameref, NMTEST_TMDATUM));
  tmd = nameref->u.tmdecl->tmdef;
  cwl = tmd->sltcw;
  assert(cwl != NULL && cwl->home_row_text != NULL);
  fprintf(ofile, "#define COPY_MFCtr");
  if (tmd->flags & TMDF_HOMEROW)
    fprintf(ofile, "\n#define " MFC_NAME " %s\n", cwl->home_row_text);
  else
    fprintf(ofile, " " MFC_NAME " = %s;", cwl->home_row_text);
}

void post_processing(void) {
  { /* Output defines for skeleton */
    rational Rval;
    unsigned int rollover, mfcsperrow;

    rtimesint(&Rsynch, SynchPer * 10, &Rval);
    fprintf(ofile, "\n#define TRN %d\n", Rval.num);
    fprintf(ofile, "#define TRD %d\n", Rval.den);
    rtimesint(&Rval, Rval.den, &Rval);
    rdivideint(&Rval, 10, &Rval);
    rtimesint(&Rval, SecondsDrift, &Rval);
    fprintf(ofile, "#define LOWLIM (%d)\n", -Rval.num/Rval.den);
    fprintf(ofile, "#define HIGHLIM (%d)\n", Rval.num/Rval.den);
    fprintf(ofile, "#define NBROW %d\n", Ncols);
    fprintf(ofile, "#define NROWMINF %d\n", SynchPer);
    rtimesint(&Rsynch, SynchPer, &Rval);
    fprintf(ofile, "#define NSECSPER %d\n#define NROWSPER %d\n",
                Rval.den, Rval.num);
    mfcsperrow = Nrows/SynchPer;
    rollover = (USHRT_MAX % mfcsperrow) + 1;
    if (rollover != mfcsperrow) {
      rollover = - rollover;
      fprintf(ofile, "#define ROLLOVER %u\n", rollover);
    }
    fprintf(ofile, "#define SYNCHVAL 0x%02X%02X\n", SynchValue & 0xFF,
                  (SynchValue>>8) & 0xFF);
    if (SynchInverted) fprintf(ofile, "#define INVSYNCH\n");
    fprintf(ofile, "#define NROWMAJF %d\n", Nrows);
    fprintf(ofile, "#define MFSECNUM %d\n", Rsynch.num);
    fprintf(ofile, "#define MFSECDEN %d\n", Rsynch.den);
    fprintf(ofile, "#define SECDRIFT %d\n", SecondsDrift);
    if (!Collecting) print_mfcopy();
  }
  Skel_copy(ofile, "data_defs", 1);
  
  /* Output main program */
  Skel_copy(ofile, "main_program", !(compile_options & CO_NO_MAIN));
    
  /* Generate pointer and proxy sharing code if necessary */
  if (Collecting) print_ptr_proxy();

  /* output remainder of skeleton */
  Skel_copy(ofile, NULL, 1);

  { /* output dbr_info (after skeleton to assure dbr.h included */
    rational Rval;
    struct nm *mfc;
    struct cw *cwl;
    tm_dac_t tmi;
    int verfd;

    mfc = find_name(MFC_NAME, 0);
    assert(name_test(mfc, NMTEST_TMDATUM));
    rtimesint(&Rsynch, SynchPer, &Rval);
    memset(&tmi.tmid, 0, sizeof(tmi.tmid));
    verfd = open( "VERSION", O_RDONLY );
    if ( verfd < 0 ) {
      compile_error( 1, "Cannot find VERSION, using '1.0'" );
      strcpy( tmi.tmid.version, "1.0" );
    } else {
      int i, rc;
      rc = read( verfd, tmi.tmid.version, 16 );
      close( verfd );
      for ( i = 0; i < rc; i++ ) {
        if ( tmi.tmid.version[i] == '\n' || tmi.tmid.version[i] == '\r' )
          break;
      }
      while ( i < 16 ) tmi.tmid.version[i++] = '\0';
    }
    memcpy( tmi.tmid.md5, md5_sig, 16 );
    tmi.nbminf = SynchPer * Ncols;
    tmi.nbrow = Ncols;
    tmi.nrowmajf = Nrows;
    tmi.nsecsper = Rval.den;
    tmi.nrowsper = Rval.num;
    cwl = mfc->u.tmdecl->tmdef->sltcw;
    assert(cwl != NULL && cwl->dnext == NULL);
    assert(cwl->slot != NULL && cwl->slot->row == 0);
    tmi.mfc_lsb = cwl->col;
    tmi.mfc_msb = cwl->col+1;
    tmi.synch = ((SynchValue & 0xFF) << 8) | ((SynchValue >> 8) & 0xFF);
    tmi.flags = SynchInverted ? TMF_INVERTED : 0;
    
    print_indent(NULL);
    fprintf(ofile, "\ntm_info_t tm_info = {\n ");
    fprintf( ofile, "  /* version: '%s' */\n ", tmi.tmid.version );
    { int i;
      for ( i = 0; i < 16; i += 8 ) {
        int j;
        for (j = 0; j < 8; j++ ) {
          fprintf( ofile, " %3d,", tmi.tmid.version[i+j] );
        }
        fprintf( ofile, "\n " );
      }
    }
    fprintf( ofile, "  /* md5: */\n " );
    { int i;
      for ( i = 0; i < 16; i += 8 ) {
        int j;
        for (j = 0; j < 8; j++ ) {
          fprintf( ofile, " 0x%02X,", tmi.tmid.md5[i+j] );
        }
        fprintf( ofile, "\n " );
      }
    }
    fprintf(ofile,  " %d, /* NBMINF */\n"
                   "  %d, /* NBROW */\n"
                   "  %d, /* NROWSMAJF */\n"
                   "  %d, /* NSECSPER */\n"
                   "  %d, /* NROWSPER */\n",
         tmi.nbminf, tmi.nbrow, tmi.nrowmajf, tmi.nsecsper, tmi.nrowsper);
    fprintf(ofile, "  %d, %d, /* MFC lsb col, msb col */\n",
                    tmi.mfc_lsb, tmi.mfc_msb);
    fprintf(ofile, "  0x%04X, /* Synch Value */\n"
                   "  %s\n};\n",
                   tmi.synch, tmi.flags ?
                     "TMF_INVERTED" :
                     "0 /* not inverted */");
    if (dacfile != NULL) {
      fwrite(&tmi, sizeof(tmi), 1, dacfile);
      fclose(dacfile);
    }
  }
  
  if ( addrfile != NULL ) {
    struct nm *datum;
    for ( datum = global_scope->names; datum != NULL;
          datum = datum->next ) {
      char *name = datum->name;
      struct declrtor *decl;
      switch ( datum->type ) {
        case NMTYPE_DATUM:
          decl = datum->u.ddecl->decl;
          break;
        case NMTYPE_TMDATUM:
          decl = datum->u.tmdecl->decl;
          break;
        default: continue;
      }
      if ( decl->flag & DCLF_ADDRDEF ) {
        fprintf( addrfile, "#define %s_Address 0x%X\n",
          name, decl->address );
      }
    }
    fclose( addrfile );
  }
}
