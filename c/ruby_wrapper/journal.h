#ifndef H_JOURNAL
#define H_JOURNAL

#include "laser_data.h"

void journal_open(const char*);

void journal_write_array_d(const char*str, int n, double*);
void journal_write_array_i(const char*str, int n, int*);
void journal_laser_data(const char*name, struct laser_data*ld);

void journal_correspondences(LDP ld);
void journal_pose(const char*, gsl_vector*v);
void journal_point(const char*, gsl_vector*v);

/// Returns journal file 
FILE *jf();

#endif