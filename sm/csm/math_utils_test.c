#include "csm_all.h"
#include "math_utils.h"

float max_acos = -100, min_acos = 100;
/*
float myacos(float x) {
	max_acos = GSL_MAX(x, max_acos);
	min_acos = GSL_MIN(x, min_acos);
	return acosf(x);
}*/
float max_asin = -100, min_asin = 100;
float myasin(float x) {
	max_asin = GSL_MAX(x, max_asin);
	min_asin = GSL_MIN(x, min_asin);
	
	float x2 = x * x;
	return x * ( 1 + x2 *( (1.0f/6) + (3.0f/40) * x2)) + (15.0f/ (48*7))* x2*x2*x2*x;
}

float myacos(float x) {
	return M_PI/2 - myasin(x);
}

void polar(double x, double y, double* restrict rho, double* restrict theta) {
	*rho = sqrt(x*x+y*y);
	if(x >= 0 && y >= 0) {
		if(x < y)
			*theta = myacos(x / *rho);
		else
			*theta = asin(y / *rho);
	} else
	if(x >= 0 && y < 0) {
		if( x < -y )
			*theta = - myacos(x / *rho);
		else
			*theta = - myasin( (-y) / *rho) ;
	} else 
	if(x < 0 && y >= 0) {
		if( (-x) < y)
			*theta = M_PI - myacos(-x / *rho);
		else
			*theta = M_PI - myasin( y / *rho);
	} else 
	if(x < 0 && y < 0) {
		if ( (-x) < (-y) )
			*theta = M_PI + myacos(-x / *rho);
		else
			*theta = M_PI + myasin(-y / *rho);
	}
}

int main() {
	double a[2] = {0, 0};
	double b[2] = {5, 5};
	double x[5][2] = {
		{0, -10},
		{  2, 1},
		{100, 1},
		{0,0},{5,5}};
	
	gsl_vector * A = vector_from_array(2,a);
	gsl_vector * B = vector_from_array(2,b);

	gsl_vector * res = gsl_vector_alloc(2);
	
	int i;
	#if 0
	for(i=0;i<5;i++){
		gsl_vector * X = vector_from_array(2,x[i]);
		
/*		projection_on_segment(A,B,X,res);*/
		
		printf("Projection of %f %f is %f %f \n", gvg(X,0),gvg(X,1),
			gvg(res,0),gvg(res,1));
	}
	#endif
	
	int errors = 0;
	double should_be_nan[2] = { 0.0 / 0.0, GSL_NAN };
	for(i=0;i<2;i++) {
		if(!isnan(should_be_nan[i])) {
			printf("#%d: isnan(%f) failed \n", i, should_be_nan[i]);
			errors++;
		}
		if(!is_nan(should_be_nan[i])) {
			printf("#%d: is_nan(%f) failed \n", i, should_be_nan[i]);
			errors++;
		}
	}
	
	double max_error = 0;
	int num = 10000;
	for(i=0;i<num;i++) {
		double theta = (i * 2 * M_PI) / (num-1);
		double x = cos(theta), y = sin(theta);
		
		double theta_est, rho;
		polar(x,y,&rho,&theta_est);
		double error = fabs( angleDiff(theta_est,theta) );
		if(error > max_error)
		printf("%d x %f y %f theta %f theta_est %f error %f \n", i, x, y, theta, theta_est, error);
		max_error = GSL_MAX(error, max_error);
	}
	
	printf("Maximum error for polar(): %lf rad = %lf deg\n", max_error, rad2deg(max_error));
	printf("Acos() called for   %f  %f\n", min_acos, max_acos);
	printf("Asin() called for   %f  %f\n", min_asin, max_asin);
	return errors;
}
