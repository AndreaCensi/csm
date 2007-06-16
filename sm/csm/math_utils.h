#ifndef H_MATH_UTILS
#define H_MATH_UTILS

#include <egsl/egsl.h>
#include <gsl/gsl_math.h>
#include "laser_data.h"

#define gvg gsl_vector_get
#define gvs gsl_vector_set


/* Sometimes I really don't understand compilers.. */ 
#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#ifndef NAN
#define NAN GSL_NAN
#endif
     
void possible_interval(
	const double *p_i_w, struct laser_data*laser_sens, 
	double max_angular_correction_deg, double max_linear_correction, int*from, int*to, int*start_cell);

void transform(const gsl_vector* point2d, const gsl_vector* pose, gsl_vector*result2d);
void transform_d(const double* point2d, const double* pose, double* result2d);

void gsl_vector_set_nan(gsl_vector*v);

double distance(const gsl_vector* a,const gsl_vector* b);
double distance_squared(const gsl_vector* a,const gsl_vector* b);

/** Returns norm of 2D point p */
double norm(const gsl_vector*p);
/** Returns norm of 2D point p */
double norm_d(const double*p);

double distance_squared_d(const double *a, const double *b);


double angleDiff(double a, double b);
double square(double x);
double deg2rad(double deg);
double rad2deg(double rad);
gsl_vector * vector_from_array(unsigned int n, double *x);
void vector_to_array(const gsl_vector*v, double*);
void copy_from_array(gsl_vector*v, double*);

int minmax(int from,int to,int x);

/** Copies n doubles from from to to */
void copy_d(const double*from, int n, double*to);
/* With doubles */
void ominus_d(const double *x, double*res);
void oplus_d(const double*x1, const double*x2, double*res);
void pose_diff_d(const double*second, const double*first, double*res);
/* With vectors */
void oplus(const gsl_vector*x1,const gsl_vector*x2, gsl_vector*res);
void ominus(const gsl_vector*x, gsl_vector*res);
void pose_diff(const gsl_vector*pose2,const gsl_vector*pose1,gsl_vector*res);

	
/** Projects (p[0],p[1]) on the LINE passing through (ax,ay)-(bx,by). If distance!=0, distance is set
to the distance from the point to the segment */
void projection_on_line_d(const double *a,
	const double *b,
	const double *p,
	double *res,
	double *distance);
	
/** Distance of x from its projection on segment a-b */
double dist_to_segment_d(const double*a, const double*b, const double*x);
double dist_to_segment_squared_d(const double*a, const double*b, const double*x);

void projection_on_segment_d(
	const double*a,
	const double*b,
	const double*x,
	      double*proj);

/** Some functions to print poses and covariances in a friendly way */
const char* friendly_pose(double*pose);
const char* gsl_friendly_pose(gsl_vector*v);
const char* egsl_friendly_pose(val pose);
const char* egsl_friendly_cov(val cov);


/** Returns Fisher's information matrix. You still have to multiply
    it by (1/sigma^2). */
val ld_fisher0(LDP ld);

int is_nan(double v);

#endif

