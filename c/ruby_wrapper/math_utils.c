#include <gsl/gsl_nan.h>

#include "math_utils.h"

void possible_interval(
	gsl_vector*p_i_w, LDP ld, 
	double maxAngularCorrectionDeg, double maxLinearCorrection, int*from, int*to) 
{
	double angleRes = (ld->max_theta-ld->min_theta)/ld->nrays;

	// Delta for the angle
	double delta = fabs(deg2rad(maxAngularCorrectionDeg)) +
	        fabs(atan(maxLinearCorrection/norm(p_i_w)));

	// Dimension of the cell range
	int range = (int) ceil(delta/angleRes);

	// To be turned into an interval of cells
	double start_theta = atan2(gvg(p_i_w,1),gvg(p_i_w,0));
	
	int start_cell  = (int)
		((start_theta - ld->min_theta) / (ld->max_theta-ld->min_theta) * ld->nrays);

	*from = GSL_MIN(ld->nrays-1, GSL_MAX(start_cell-range,0));
	*to =   GSL_MIN(ld->nrays-1, GSL_MAX(start_cell+range,0));
}


void transform(const gsl_vector* p, const gsl_vector* x, gsl_vector*res) {
	double theta = gvg(x,2);
	double c = cos(theta); double s = sin(theta);
	gsl_vector_set(res, 0, c * gvg(p,0) -s*gvg(p,1) + gvg(x,0));
	gsl_vector_set(res, 1, s * gvg(p,0) +c*gvg(p,1) + gvg(x,1));
}

void gsl_vector_set_nan(gsl_vector*v) {
	gvs(v,0,GSL_NAN);
	gvs(v,1,GSL_NAN);
}


double distance(gsl_vector* a, gsl_vector* b) {
	double x = gvg(a,0)-gvg(b,0);
	double y = gvg(a,1)-gvg(b,1);
	return sqrt(x*x+y*y);
}

double norm(const gsl_vector*a){
	double x = gvg(a,0);
	double y = gvg(a,1);
	return sqrt(x*x+y*y);
}

double deg2rad(double deg) {
	return deg * M_PI / 180;
}

double rad2deg(double rad) {
	return rad / M_PI * 180;	
}

gsl_vector * vector_from_array(int n, double *x) {
	gsl_vector * v = gsl_vector_calloc(n);
	int i;
	for(i=0;i<n;i++)
		gvs(v,i,x[i]);
	
	return v;
}

