#include <gsl/gsl_nan.h>

#include "math_utils.h"

void possible_interval(
	gsl_vector*p_i_w, LDP ld, 
	double maxAngularCorrectionDeg, double maxLinearCorrection, int*from, int*to, int*start_cell) 
{
	double angleRes = (ld->max_theta-ld->min_theta)/ld->nrays;

	// Delta for the angle
	double delta = fabs(deg2rad(maxAngularCorrectionDeg)) +
	        fabs(atan(maxLinearCorrection/norm(p_i_w)));

	// Dimension of the cell range
	int range = (int) ceil(delta/angleRes);

	// To be turned into an interval of cells
	double start_theta = atan2(gvg(p_i_w,1),gvg(p_i_w,0));
	
	*start_cell  = (int)
		((start_theta - ld->min_theta) / (ld->max_theta-ld->min_theta) * ld->nrays);

	*from = minmax(0,ld->nrays-1, *start_cell-range);
	*to =   minmax(0,ld->nrays-1, *start_cell+range);
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

gsl_vector * vector_from_array(unsigned int n, double *x) {
	gsl_vector * v = gsl_vector_calloc(n);
	unsigned int i;
	for(i=0;i<n;i++)
		gvs(v,i,x[i]);
	
	return v;
}

void vector_to_array(const gsl_vector*v, double*x){
	size_t i; 
	for(i=0;i<v->size;i++)
		x[i] = gvg(v,i);
}


void oplus(const gsl_vector*x1,const gsl_vector*x2, gsl_vector*res) {
	double c = cos(gvg(x1,2));
	double s = sin(gvg(x1,2));
	gvs(res,0,  gvg(x1,0)+c*gvg(x2,0)-s*gvg(x2,1));
	gvs(res,1,  gvg(x1,1)+s*gvg(x2,0)+c*gvg(x2,1));
	gvs(res,2,  gvg(x1,2)+gvg(x2,2));
}

void ominus(const gsl_vector*x, gsl_vector*res) {
	double c = cos(gvg(x,2));
	double s = sin(gvg(x,2));
	gvs(res,0,  -c*gvg(x,0)-s*gvg(x,1));
	gvs(res,1,   s*gvg(x,0)-c*gvg(x,1));
	gvs(res,2,  -gvg(x,2));
}

void pose_diff(const gsl_vector*pose2,const gsl_vector*pose1,gsl_vector*res) {
	gsl_vector* temp = gsl_vector_alloc(3);
	ominus(pose1, temp);
	oplus(temp, pose2, res);
	gsl_vector_free(temp);
}


double minmax(int from,int to,int x){
	return GSL_MAX(GSL_MIN(x,to),from);
}

double square(double x) {
	return x*x;
}

double angleDiff(double a, double b) {
	double t = a - b;
	while(t<-M_PI) t+= 2*M_PI;
	while(t>M_PI)  t-= 2*M_PI;
	return t;
}


