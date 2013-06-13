#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "disc_cmd.h"

int get_type(char *buf, int *type) {
  char code[10];
  int i = 0;

  while (isspace(*buf)) buf++;
  if (sscanf(buf, "%[ABCELNPRUST]s,", code) == 1) {
    if (code[i] == 'S') {
      ++i;
      switch (code[i++]) {
        case 'T':
          if (code[i] == 'E')
            if ((code[++i] != 'P') || (code[++i] != '\0')) break;
            else *type = STEP;
          else if ((code[i++] != 'R') || (code[i++] != 'B') || (code[i] != '\0'))
            break;
          else *type = STRB;
          return(0);
        case 'E':
          if (code[i] == 'L')
            if ((code[++i] != 'E') || code[++i] != 'C' || code[++i] != 'T' ||
                (code[++i] != '\0')) break;
            else {
              *type= SELECT;
              return(0);
            }
          else if ((code[i++] != 'T') || (code[i] != '\0')) break;
          *type= SET;
          return(0);
        case 'P':
          if ((code[i++] != 'A') || (code[i++] != 'R') ||
             (code[i++] != 'E') || (code[i] != '\0')) break;
          *type = SPARE;
          return(0);
      }
    } else if (strcmp(code,"UNSTRB") == 0) {
      *type = UNSTRB;
      return 0;
    }
  }
  return(-1);
}

int get_line(FILE *fp, char *buf) {
  char *p;

  for (p = fgets(buf, 128, fp); p != NULL; )
    if ((*p == '\0') || (*p == ';')) p = fgets(buf, 128, fp);
    else if (isalnum(*p)) break;
    else p++;
  if (p == NULL) return(-1);
  return(0);
}
