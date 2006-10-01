#include <gsl/gsl_nan.h>

#include "math_utils.h"

void possible_interval(
	gsl_vector*p_i_w, struct laser_data*laser_sens, 
	double maxAngularCorrectionDeg, double maxLinearCorrection, int*from, int*to) 
{
		
		
}

#define gvg gsl_vector_get
#define gvs gsl_vector_set

void transform(const gsl_vector* p, const gsl_vector* x, gsl_vector*res) {
	double theta = gvg(x,3);
	double c = cos(theta); double s = sin(theta);
	gsl_vector_set(res, 0, c * gvg(p,0) -s*gvg(p,1) + gvg(x,0));
	gsl_vector_set(res, 1, s * gvg(p,0) +c*gvg(p,1) + gvg(x,1));
}

void gsl_vector_set_nan(gsl_vector*v) {
	gvs(v,0,GSL_NAN);
	gvs(v,1,GSL_NAN);
}
