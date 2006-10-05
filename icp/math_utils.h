#ifndef H_MATH_UTILS
#define H_MATH_UTILS

#include <gsl/gsl_math.h>
#include "laser_data.h"

void possible_interval(
	gsl_vector*p_i_w, struct laser_data*laser_sens, 
	double maxAngularCorrectionDeg, double maxLinearCorrection, int*from, int*to, int*start_cell);

void transform(const gsl_vector* point, const gsl_vector* x, gsl_vector*result);

void gsl_vector_set_nan(gsl_vector*v);

double distance(gsl_vector* a,gsl_vector* b);
double norm(const gsl_vector*);

double angleDiff(double a, double b);
double square(double x);
double deg2rad(double deg);
double rad2deg(double rad);
gsl_vector * vector_from_array(unsigned int n, double *x);

double minmax(int from,int to,int x);

void oplus(const gsl_vector*x1,const gsl_vector*x2, gsl_vector*res);
void ominus(const gsl_vector*x, gsl_vector*res);
void pose_diff(const gsl_vector*pose2,const gsl_vector*pose1,gsl_vector*res);


#define gvg gsl_vector_get
#define gvs gsl_vector_set

#endif