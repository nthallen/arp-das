#include <stdio.h>
#include "depfile.h"
#include "err.h"

static FILE *depfile_fp;

void OpenDepFile() {
  if (DepFile != NoKey) {
    char *dfname = StringTable(GetClpValue(DepFile,0));
    depfile_fp = fopen(dfname, "w");
    if (depfile_fp == 0) {
      message(ERROR, "Unable to open dependency output file", 0, 0);
    }
  }
}

void WriteDepFile(const char *IFile) {
  if (depfile_fp) {
    fprintf(depfile_fp, " %s", IFile);
  }
}

void CloseDepFile() {
  if (depfile_fp) {
    fprintf(depfile_fp, "\n");
    fclose(depfile_fp);
    depfile_fp = 0;
  }
}
