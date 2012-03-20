/**
 * \file csv_file.cc
 * These classes definee the runtime for extraction to CSV output
 */
#include <malloc.h>
#include "csv_file.h"
#include "nortlib.h"
#include "nl_assert.h"

csv_col::csv_col(const char *colname, const char *fmt) {
  cname = colname;
  format = fmt;
  dsval = 0;
  dsval_size = 0;
}

csv_col::~csv_col() {
  if (dsval) free(dsval);
}

const char *csv_col::output() {
  return dsval ? dsval : "";
}

const char *csv_col::header() {
  return cname ? cname : "";
}

void csv_col::dsval_resize(int newsize) {
  char *newbuf;
  dsval_size = newsize;
  newbuf = (char *)realloc(dsval, dsval_size);
  if (newbuf == NULL) {
    nl_error(3, "Memory allocation failure in csv_col::set(double)");
  }
  dsval = newbuf;
}

void csv_col::set(double dval) {
  int sz = snprintf(dsval, dsval_size, format, dval);
  if (sz >= dsval_size) {
    dsval_resize(sz+5);
    sz = snprintf(dsval, dsval_size, format, dval);
    nl_assert(sz < dsval_size);
  }
}

void csv_col::set(const char *tval) {
  int sz = snprintf(dsval, dsval_size, "%s", tval);
  if (sz >= dsval_size) {
    dsval_resize(sz+5);
    sz = snprintf(dsval, dsval_size, "%s", tval);
    nl_assert(sz < dsval_size);
  }
}

void csv_col::reset() {
  if (dsval) dsval[0] = '\0';
}


csv_file::csv_file(const char *name, int n_cols) {
  filename = name;
  cols.resize(n_cols);
  time_set = false;
  fp = fopen( filename, "w" );
  if (fp == NULL)
    nl_error(3, "Cannot open output file %s", filename);
}

csv_file::~csv_file() {
  int i;
  if (time_set) flush_row();
  fclose(fp);
  for (i = 0; i < cols.size(); ++i) {
    if ( cols[i] ) delete cols[i];
  }
}

void csv_file::init_col(int col_num, const char *colname, const char *fmt) {
  if (time_set)
    nl_error(3, "csv_file::init_col() after time_set");
  if (col_num < 0 || col_num >= cols.size())
    nl_error(3, "col_num %d out of range in csv_file::init_col", col_num);
  if (cols[col_num])
    nl_error(3, "Illegal redefinition of column %d in csv_file::init_col",
      col_num);
  cols[col_num] = new csv_col(colname, fmt);
}

void csv_file::set_time(double T) {
  if (cols[0] == 0)
    cols[0] = new csv_col("Time", "%.0lf");
  if (time_set) {
    if (T != cur_time) flush_row();
  } else {
    flush_headers();
  }
  time_set = true;
  cur_time = T;
  cols[0]->set(T);
}

void csv_file::flush_headers() {
  int i;
  nl_assert(cols[0]);
  fprintf(fp, "%s", cols[0]->header());
  for (i = 1; i < cols.size(); ++i) {
    fprintf(fp, ",%s", cols[i] ? cols[i]->header() : "");
  }
  fprintf(fp, "\n");
}

void csv_file::flush_row() {
  int i;
  fprintf(fp, "%s", cols[0]->output() );
  for (i = 1; i < cols.size(); ++i) {
    fprintf(fp, ",");
    if (cols[i]) {
      fprintf(fp, "%s", cols[i]->output() );
      cols[i]->reset();
    }
  }
  fprintf(fp, "\n");
}

/**
 * This interface is not for column zero (Time)
 */
void csv_file::set_col(int col_num, double dval) {
  if (col_num < 1 || col_num >= cols.size()) {
    nl_error(3, "col_num %d out of range in csv_file::set_col", col_num);
  } else if (cols[col_num] == NULL) {
    nl_error(3, "column %d undefined in csv_file::set_col", col_num);
  } else cols[col_num]->set(dval);
}

void csv_file::set_col(int col_num, const char *sval) {
  if (col_num < 1 || col_num >= cols.size()) {
    nl_error(3, "col_num %d out of range in csv_file::set_col", col_num);
  } else if (cols[col_num] == NULL) {
    nl_error(3, "column %d undefined in csv_file::set_col", col_num);
  } else cols[col_num]->set(sval);
}
