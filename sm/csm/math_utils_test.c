#include "csm_all.h"
#include "math_utils.h"

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
			printf("#%d: isnan(%f) failed \n", should_be_nan[i]);
			errors++;
		}
		if(!is_nan(should_be_nan[i])) {
			printf("#%d: is_nan(%f) failed \n", should_be_nan[i]);
			errors++;
		}
	}
	
	return errors;
}
