#include <math.h>
#include "easy_gsl.h"
#include "easy_gsl_macros.h"

int main() {
	
	egsl_push();
	
	val R = rot(M_PI/2);

	egsl_print("R", R);
	
	
	double p[2] = {1,2};
	
	val vp = egsl_vFa(2,p);
	
	egsl_print("vp", vp);
	
	val vrot = m(R, vp); 
	
	egsl_print("vrot", vrot);
	
	egsl_pop();
}

