#include "csm_all.h"
#include <time.h>

#include "math_utils.h"


inline float myasin(float x) {
	const float threshold = .52;
	if(x > threshold) {
		static float z, der0, der1,der2,der3;
		static int first = 1;
		if(first) {
			z = 0.5* (threshold + 0.707);
			der0 = asin(z);
			float M = 1-z*z;
			der1 = 1 / sqrt(M);
			der2 = (sqrt(M) * z) / (M*M);
			der3 = (sqrt(M) / ( M * M ) + (3 * (z*z) * sqrt(M) ) / (M * M * M) )  ;
			first = 0;
			printf("%f %f  %f %f \n", der0, der1, der2, der3);
		}
		float m = x-z;
		return der0 + m * ( der1 +  m * ( (der2/2) +   m * (der3/6) ) );
	}
	else{
		float x2 = x * x;
		float der1 = 1;
		float der3 = 1.0f/6;
		float der5 = 3.0f/40;
		float der7 = 15.0f/ (48*7);
		return x * ( der1 + x2 *( der3 + x2 * (der5  + der7 * x2 )));
	}
}

#define myacos(x)  ( ((float) (M_PI/2) ) - myasin(x))
inline void polar(float x, float y, double* restrict rho, double* restrict theta) {
	*rho = sqrtf(x*x+y*y);
	if(x > 0) {
		if(y > 0) {
			if(x < y)
				*theta = myacos(x / *rho);
			else
				*theta = myasin(y / *rho);
		} else {
			if( x < -y )
				*theta = - myacos(x / *rho);
			else
				*theta = - myasin( (-y) / *rho) ;
		} 
	} else  {
		if(y > 0) {
			if( (-x) < y)
				*theta = ((float)M_PI) - myacos(-x / *rho);
			else
				*theta = ((float)M_PI) - myasin( y / *rho);
		} else {
			if ( (-x) < (-y) )
				*theta = ((float)M_PI) + myacos(-x / *rho);
			else
				*theta = ((float)M_PI) + myasin(-y / *rho);
		}
	}
}

inline void polar2(double x, double y, double* restrict rho, double* restrict theta) {
	*rho = sqrt(x*x+y*y);
	*theta = atan2(y,x);
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

	int NUM2 = 1000; int k;
	float c[num], s[num];
	for(i=0;i<num;i++) {
		double theta = (i * 2 * M_PI) / (num-1);
		c[i]= cos(theta); s[i] = sin(theta);
	}
	
	clock_t start1 = clock();
	double thetaf,rhof;
	for(k=0;k<NUM2;k++)
	for(i=0;i<num;i++) {
		polar(c[i],s[i],&rhof, &thetaf);
	}
	clock_t end1 = clock();
	
	clock_t start2 = clock();
	double rho,theta;
	for(k=0;k<NUM2;k++)
	for(i=0;i<num;i++) {
		polar2(c[i],s[i],&rho, &theta);
	}
	clock_t end2 = clock();
	
	float seconds1 = (end1-start1)/((float)CLOCKS_PER_SEC);
	float seconds2 = (end2-start2)/((float)CLOCKS_PER_SEC);
	
	printf("  polar: %f   polar2: %f\n", seconds1, seconds2);
	
	printf("Maximum error for polar(): %lf rad = %lf deg\n", max_error, rad2deg(max_error));
/*	printf("Acos() called for   %f  %f\n", min_acos, max_acos);
	printf("Asin() called for   %f  %f\n", min_asin, max_asin);
*/	return errors;
}
