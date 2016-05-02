#ifndef DEPFILE_H_INCLUDED
#define DEPFILE_H_INCLUDED

#include "clp.h"
#include "csm.h"

void OpenDepFile();
void WriteDepFile(const char *IFile);
void CloseDepFile();

#endif

