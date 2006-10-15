#include <gsl/gsl_nan.h>

#include "math_utils.h"

inline double minmax(int from,int to,int x){
	return GSL_MAX(GSL_MIN(x,to),from);
}

void possible_interval(
	const gsl_vector*p_i_w, LDP ld, 
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

int distance_counter = 0;
double distance(const gsl_vector* a, const gsl_vector* b) {
	distance_counter++;
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
	gsl_vector * v = gsl_vector_alloc(n);
	unsigned int i;
	for(i=0;i<n;i++)
		gvs(v,i,x[i]);
	
	return v;
}
void copy_from_array(gsl_vector*v, double*x) {
	size_t i; 
	for(i=0;i<v->size;i++)
		gsl_vector_set(v,i, x[i]);
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



double square(double x) {
	return x*x;
}

double angleDiff(double a, double b) {
	double t = a - b;
	while(t<-M_PI) t+= 2*M_PI;
	while(t>M_PI)  t-= 2*M_PI;
	return t;
}


/// Computes the projection of x onto the line which goes through a-b
void projection_on_line(const gsl_vector*a,const gsl_vector*b,const gsl_vector*x,
	gsl_vector * proj) 
{
	double t0 = gvg(a,0)-gvg(b,0);
	double t1 = gvg(a,1)-gvg(b,1);
	
	double alpha = atan2(t1,t0) + M_PI/2;
	double rho = cos(alpha)*gvg(a,0)+sin(alpha)*gvg(a,1);
	
	double c = cos(alpha); double s = sin(alpha);
	double px =  gvg(x,0); double py = gvg(x,1);
	gvs(proj, 0, c*rho + s*s*px - c*s*py );
	gvs(proj, 1, s*rho - c*s*px + c*c*py );	
}

/// Computes the projection of x onto the segment a-b
void projection_on_segment(const gsl_vector*a,const gsl_vector*b,const gsl_vector*x,
	gsl_vector * proj ) {
	projection_on_line(a,b,x,proj);
	if ((gvg(proj,0)-gvg(a,0))*(gvg(proj,0)-gvg(b,0)) +
	    (gvg(proj,1)-gvg(a,1))*(gvg(proj,1)-gvg(b,1)) < 0 ) {
		// the projection is inside the segment
	} else 
		if(distance(a,x)<distance(b,x)) 
			gsl_vector_memcpy(proj,a);
		else
			gsl_vector_memcpy(proj,b);
}

double dist_to_segment(const gsl_vector*a,const gsl_vector*b,const gsl_vector*x) {
	gsl_vector * projection = gsl_vector_alloc(2);
	projection_on_segment(a,b,x,projection);
	double dist = distance(projection, x);
	gsl_vector_free(projection);
	return dist;
}

char tmp_buf[100];
const char* gsl_friendly_pose(gsl_vector*v) {
	sprintf(tmp_buf, "(%4.2f mm, %4.2f mm, %4.4f deg)",
		1000*gvg(v,0),1000*gvg(v,1),rad2deg(gvg(v,2)));
	return tmp_buf;
}

const char* egsl_friendly_pose(val v) {
	sprintf(tmp_buf, "(%4.2f mm, %4.2f mm, %4.4f deg)",
		1000*egsl_atv(v,0),
		1000*egsl_atv(v,1),
		rad2deg(egsl_atv(v,2)));
	return tmp_buf;
}

const char* egsl_friendly_cov(val cov) {
	
	double limit_x  = 2 * sqrt(egsl_atm(cov, 0, 0));
	double limit_y  = 2 * sqrt(egsl_atm(cov, 1, 1));
	double limit_th = 2 * sqrt(egsl_atm(cov, 2, 2));
	
	sprintf(tmp_buf, "(+- %4.2f mm,+- %4.2f mm,+- %4.4f deg)",
		1000*limit_x,
		1000*limit_y,
		rad2deg(limit_th));
	return tmp_buf;
}

