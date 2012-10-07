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
    bool warned;
};

class csv_file {
  public:
    csv_file(const char *name, unsigned int n_cols,
        const char *nan_text = NULL, int json = 0);
    ~csv_file();
    void init();
    void init_col(unsigned int col_num, const char *colname,
                  const char *fmt = NULL);
    void set_time(double T);
    void set_col(unsigned int col_num, double dval);
    void set_col(unsigned int col_num, const char *sval);
    static void terminate_on_write_err(int sig);
    static const char *nan;
  private:
    void flush_headers();
    void flush_row();
    std::vector<csv_col*> cols;
    double cur_time;
    bool time_set;
    bool json;
    const char *filename;
    FILE *fp;
};

#endif
