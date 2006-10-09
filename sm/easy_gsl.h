#ifndef H_EASY_GSL
#define H_EASY_GSL

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

typedef int val;

void egsl_push();
void egsl_pop();
void egsl_print(const char*str, val);


val egsl_zeros(int rows, int columns);
val egsl_scale(double, val);
val egsl_sum(val, val);
val egsl_sum3(val, val, val);
val egsl_mult(val, val);
val egsl_transpose(val);
val egsl_inverse(val);
double egsl_norm(val);

double egsl_atv(val, int i);
double egsl_atm(val, int i, int j);

val egsl_sub(val,val);
val egsl_sum(val v1,val v2);
val egsl_compose_col(val v1, val v2);

void egsl_add_to(val v1, val v2);
void egsl_add_to_col(val v1, int j, val v2);

val egsl_vers(double theta);
val egsl_rot(double theta);

val egsl_vFa(int rows, const double*);
val egsl_vFda(int rows, int columns, const double*);
void egsl_v2a(val, double**);
void egsl_v2da(val, double*);

val egsl_vFgslv(const gsl_vector*);
val egsl_vFgslm(const gsl_matrix*);
gsl_matrix* egsl_v2gslm(val);

#endif