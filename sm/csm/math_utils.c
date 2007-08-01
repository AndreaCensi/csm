#include <gsl/gsl_nan.h>

#include "csm_all.h"

int minmax(int from,int to,int x) {
	return GSL_MAX(GSL_MIN(x,to),from);
}

void possible_interval(
	const double *p_i_w, LDP ld, 
	double max_angular_correction_deg, double max_linear_correction, int*from, int*to, int*start_cell) 
{
	double angleRes = (ld->max_theta-ld->min_theta)/ld->nrays;

	/* Delta for the angle */
	double delta = fabs(deg2rad(max_angular_correction_deg)) +
	        fabs(atan(max_linear_correction/norm_d(p_i_w)));

	/* Dimension of the cell range */
	int range = (int) ceil(delta/angleRes);

	/* To be turned into an interval of cells */
	double start_theta = atan2(p_i_w[1], p_i_w[0]);
	
	*start_cell  = (int)
		((start_theta - ld->min_theta) / (ld->max_theta-ld->min_theta) * ld->nrays);

	*from = minmax(0,ld->nrays-1, *start_cell-range);
	*to =   minmax(0,ld->nrays-1, *start_cell+range);
}



void transform_d(const double point2d[2], const double pose[3], double result2d[2]) {
	double theta = pose[2];
	double c = cos(theta); double s = sin(theta);
	result2d[0] = pose[0] + c * point2d[0] - s * point2d[1];
	result2d[1] = pose[1] + s * point2d[0] + c * point2d[1];
}

int distance_counter = 0;

double distance_squared_d(const double a[2], const double b[2]) {
	distance_counter++;
	double x = a[0]-b[0];
	double y = a[1]-b[1];
	return x*x + y*y;
}

double distance_d(const double a[2], const double b[2]) {
	return sqrt(distance_squared_d(a,b));
}


int is_nan(double v) {
	return v == v ? 0 : 1;
}

int any_nan(const double *d, int n) {
	int i; for(i=0;i<n;i++) 
		if(is_nan(d[i]))
			return 1;
	return 0;
}

double norm_d(const double p[2]) {
	return sqrt(p[0]*p[0]+p[1]*p[1]);
}

double deg2rad(double deg) {
	return deg * (M_PI / 180);
}

double rad2deg(double rad) {
	return rad * (180 / M_PI);	
}

void copy_d(const double*from, int n, double*to) {
	int i; for(i=0;i<n;i++) to[i] = from[i];
}

void ominus_d(const double x[3], double res[3]) {
	double c = cos(x[2]);
	double s = sin(x[2]);
	res[0] = -c*x[0]-s*x[1];
	res[1] =  s*x[0]-c*x[1];
	res[2] = -x[2];
}

/** safe if res == x1 */
void oplus_d(const double x1[3], const double x2[3], double res[3]) {
	double c = cos(x1[2]);
	double s = sin(x1[2]);
	double x = x1[0]+c*x2[0]-s*x2[1];
	double y = x1[1]+s*x2[0]+c*x2[1];
 	double theta = x1[2]+x2[2];
	res[0]=x;
	res[1]=y;
	res[2]=theta;
}

void pose_diff_d(const double pose2[3], const double pose1[3], double res[3]) {
	double temp[3];
	ominus_d(pose1, temp);
	oplus_d(temp, pose2, res);
	
	while(res[2] > +M_PI) res[2] -= 2*M_PI;
	while(res[2] < -M_PI) res[2] += 2*M_PI;
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

void projection_on_line_d(const double a[2],
	const double b[2],
	const double p[2],
	double res[2], double *distance)
{
	double t0 = a[0]-b[0];
	double t1 = a[1]-b[1];
	double one_on_r = 1 / sqrt(t0*t0+t1*t1);
	/* normal */
	double nx = t1  * one_on_r ;
	double ny = -t0 * one_on_r ;
	double c= nx, s = ny; 
	double rho = c*a[0]+s*a[1];

	res[0] =   c*rho + s*s*p[0] - c*s*p[1] ;
	res[1] =   s*rho - c*s*p[0] + c*c*p[1] ;	
	
	if(distance)
		*distance = fabs(rho-(c*p[0]+s*p[1]));
}

void projection_on_segment_d(
	const double a[2],
	const double b[2],
	const double x[2],
	double proj[2]) 
{
	projection_on_line_d(a,b,x,proj,0);
	if ((proj[0]-a[0])*(proj[0]-b[0]) +
	    (proj[1]-a[1])*(proj[1]-b[1]) < 0 ) {
		/* the projection is inside the segment */
	} else 
		if(distance_squared_d(a,x) < distance_squared_d(b,x)) 
			copy_d(a,2,proj);
		else
			copy_d(b,2,proj);
}


double dist_to_segment_squared_d(const double a[2], const double b[2], const double x[2]) {
	double projection[2];
	projection_on_segment_d(a, b, x, projection);
	double distance_sq_d = distance_squared_d(projection, x);
	return distance_sq_d;
}

double dist_to_segment_d(const double a[2], const double b[2], const double x[2]) {
	double proj[2]; double distance;
	projection_on_line_d(a,b,x,proj, &distance);
	if ((proj[0]-a[0])*(proj[0]-b[0]) +
	    (proj[1]-a[1])*(proj[1]-b[1]) < 0 ) {
		/* the projection is inside the segment */
		return distance;
	} else 
		return sqrt(GSL_MIN( distance_squared_d(a,x), distance_squared_d(b,x)));
}

int count_equal(const int*v, int n, int value) {
	int num = 0, i;
	for(i=0;i<n;i++) if(value == v[i]) num++;
	return num;
}

double normalize_0_2PI(double t) {
	if(is_nan(t)) {
		sm_error("Passed NAN to normalize_0_2PI().\n");
		return GSL_NAN;
	}
	while(t<0) t+=2*M_PI;
	while(t>=2*M_PI) t-=2*M_PI;
	return t;
}

static char tmp_buf[100];
const char* friendly_pose(double*pose) {
	sprintf(tmp_buf, "(%4.2f mm, %4.2f mm, %4.4f deg)",
		1000*pose[0],1000*pose[1],rad2deg(pose[2]));
	return tmp_buf;
}
