#include <math.h>
#include <gsl/gsl_math.h>
#include "egsl_macros.h"

int main() {	
	egsl_push();
	
		/* Creates a vector from a double array */
		double p[2] = {1,2};
		val vp = egsl_vFa(2,p);	
		
		/* Creates a matrix from a double array */
		double md[4] = {
			1, 2,
			0, -1
		};
		val m = egsl_vFda(2,2,md);

		/* Creates a rotation matrix */
		val R = rot(M_PI/2);
		
		/* Multiplies the three together */
		val vrot = m3(R, m, vp); 
	
		/* Displays the results */
		egsl_print("R", R);
		egsl_print("vp", vp);
		egsl_print("vrot", vrot);
	
		/* Create a semidifinite matrix (symmetric part of R) */
		val A = sc(0.5, sum(m, tr(m)) );
 
 		/* Displays spectrum */
		egsl_print("A",A);
		egsl_print_spectrum("A",A);
		
	egsl_pop();
	
	return 0;
}

