#include <math.h>
#include <egsl_macros.h>

#include "laser_data.h"
#include "math_utils.h"

// For details about the Fisher's information matrix for localization,
// please see this paper: http://purl.org/censi/2006/accuracy

val ld_fisher0(LDP ld) {
	val fim   = zeros(3,3);
	int i;
	for(i=0;i<ld->nrays;i++) {
		if(!ld->alpha_valid[i]) continue;
		
		double alpha = ld->alpha[i];
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
			c*z *t*r, s*z*t*r, t*r *t*r 
		};
		
		egsl_push();
			val k = egsl_vFda(3,3,fim_k);
			add_to(fim, k);
		egsl_pop();
	}
	return fim;
}
