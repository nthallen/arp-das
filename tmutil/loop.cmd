%{
  #include <stdio.h>

  void savelog(const char *data) {
    FILE *fp = fopen("saverun.log", "a");
    if (fp) {
      fprintf(fp, "%s\n", data);
      fclose(fp);
    } else {
      nl_error(2, "Unable to write to saverun.log");
    }
  }
%}

&command
  : SaveLog %s * { savelog($2); }
  ;
