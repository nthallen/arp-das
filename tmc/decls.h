void int_type(struct typparts *tp, char *text, unsigned int type);
unsigned int start_st_un(struct st_un *su, char *text,
            unsigned int type, unsigned int dclt);
unsigned int end_st_un(struct typparts *tp, struct st_un *su,
        char *pre, struct sttmnt *decls, char *post);
void set_typpts(struct typparts *tp, unsigned int type, unsigned int size,
      char *text, struct tmtype *tmt);
