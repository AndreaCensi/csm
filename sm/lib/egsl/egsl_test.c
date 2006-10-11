#include <math.h>

#include "egsl_macros.h"

int main() {	
	egsl_push();
	
		val R = rot(M_PI/2);

		double p[2] = {1,2};
		val vp = egsl_vFa(2,p);	

		val vrot = m(R, vp); 
	
		egsl_print("R", R);
		egsl_print("vp", vp);
		egsl_print("vrot", vrot);
	
	egsl_pop();
}

