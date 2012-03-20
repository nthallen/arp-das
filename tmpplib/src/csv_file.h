#ifndef CSV_FILE_H_INCLUDED
#define CSV_FILE_H_INCLUDED
#include <vector>
#include <stdio.h>

class csv_col {
  public:
    csv_col(const char *colname, const char *fmt = NULL);
    ~csv_col();
    const char *output();
    const char *header();
    void set(double dval);
    void set(const char *tval);
    void reset();
  protected:
    void dsval_resize(int newsize);
    const char *cname;
    char *dsval;
    int dsval_size;
    const char *format;
};

class csv_file {
  public:
    csv_file(const char *name, int n_cols);
    ~csv_file();
    void init_col(int col_num, const char *colname, const char *fmt = NULL);
    void set_time(double T);
    void set_col(int col_num, double dval);
    void set_col(int col_num, const char *sval);
  private:
    void flush_headers();
    void flush_row();
    std::vector<csv_col*> cols;
    double cur_time;
    bool time_set;
    const char *filename;
    FILE *fp;
};

#endif
