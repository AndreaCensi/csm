#ifndef H_MATH_UTILS
#define H_MATH_UTILS

#include <gsl/gsl_math.h>
#include "laser_data.h"

void possible_interval(
	gsl_vector*p_i_w, struct laser_data*laser_sens, 
	double maxAngularCorrectionDeg, double maxLinearCorrection, int*from, int*to);

void transform(const gsl_vector* point, const gsl_vector* x, gsl_vector*result);

void gsl_vector_set_nan(gsl_vector*v);

double distance(gsl_vector* a,gsl_vector* b);
double norm(const gsl_vector*);

double deg2rad(double deg);
double rad2deg(double rad);
gsl_vector * vector_from_array(int n, double *x);



#define gvg gsl_vector_get
#define gvs gsl_vector_set

#endif