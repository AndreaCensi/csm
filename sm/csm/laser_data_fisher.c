#include <math.h>
#include <egsl/egsl_macros.h>

#include "csm_all.h"

/** 
	This computes the Fisher Information Matrix, in robot coordinates.
	Uses field 'true_alpha' (and 'theta', 'readings').  
	
	For details about the the Fisher Information Matrix,
	please see this paper: http://purl.org/censi/2006/accuracy
*/

val ld_fisher0(LDP ld) {
	val fim   = zeros(3,3);
	int i;
	for(i=0;i<ld->nrays;i++) {		
		double alpha = ld->true_alpha[i];
		if(is_nan(alpha)) continue;
		
		double theta = ld->theta[i];
		double beta  = alpha-theta;
		
		double r = ld->readings[i];
		double c = cos(alpha);
		double s = sin(alpha);
		
		double z = 1 / cos(beta);
		double t = tan(beta);
		
		double fim_k[9] ={
			c*c*z*z,    c*s*z*z,   c*z *t*r,
			c*s*z*z,    s*s*z*z,   s*z *t*r, 
			c*z*t*r,    s*z*t*r,   t*r *t*r 
		};
		
		egsl_push();
			val k = egsl_vFda(3,3,fim_k);
			add_to(fim, k);
		egsl_pop();
	}
	return fim;
}
