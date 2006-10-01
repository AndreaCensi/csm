#ifndef H_MATH_UTILS
#define H_MATH_UTILS

#include "laser_data.h"

void possible_interval(
	gsl_vector*p_i_w, struct laser_data*laser_sens, 
	double maxAngularCorrectionDeg, double maxLinearCorrection, int*from, int*to);

void transform(const gsl_vector* point, const gsl_vector* x, gsl_vector*result);

void gsl_vector_set_nan(gsl_vector*v);



#endif