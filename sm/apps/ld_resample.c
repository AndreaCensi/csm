#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>

#include <math.h>
#include <options/options.h>


#include "../csm/csm_all.h"

struct {
	double interval;
	int seed;
}p ;

LDP ld_resample(LDP ld);

gsl_rng * rng;

int main(int argc, const char ** argv) {
	sm_set_program_name(argv[0]);
	
	
	struct option* ops = options_allocate(3);
	options_double(ops, "interval", &p.interval, sqrt(2.0), " 1 = no resampling");
		
	if(!options_parse_args(ops, argc, argv)) {
		options_print_help(ops, stderr);
		return -1;
	}

	int count = -1;
	LDP ld;
	while( (ld = ld_read_smart(stdin))) {
		count++;
		
		if(!ld_valid_fields(ld))  {
			sm_error("Invalid laser data (#%d in file)\n", count);
			continue;
		}
		
/*		if(count & 1) {*/
			LDP ld2 = ld_resample(ld);
			ld_write_as_json(ld2, stdout);
			ld_free(ld2);
			ld_free(ld);
/*		} else {
			ld_write_as_json(ld, stdout);
			ld_free(ld);
		}*/

		count++;
	}
	
	return 0;
}

LDP ld_resample(LDP ld) {
	/* FIXME magic number */
	int n = (int) (floor(ld->nrays / p.interval));
	
	LDP ld2 = ld_alloc_new(n);	
	int k;
	for(k=0;k<n;k++) {
		double index = k * p.interval;
		int i = (int) floor(index);
		int j = i + 1;
		double a = 1 - (index - i);
	
		
		if(  (j>= ld->nrays) || !ld->valid[i] || !ld->valid[j]) {
			ld2->valid[k] = 0;
			ld2->readings[k] = NAN;
			ld2->alpha_valid[k] = 0;
			ld2->alpha[k] = NAN;
			ld2->theta[k] = ld->theta[i];
		} else {
			ld2->theta[k] = a * ld->theta[i] + (1-a) * ld->theta[j];
			
	
			if(is_nan(ld2->theta[k])) {
				sm_debug("Hey, k=%d theta[%d]=%f theta[%d]=%f\n",
					k,i,ld->theta[i],j,ld->theta[j]);
			}

			ld2->readings[k] = a * ld->readings[i] + (1-a) * ld->readings[j];
			ld2->valid[k] = 1;
		}
		
/*		sm_debug("k=%d index=%f i=%d a=%f valid %d reading %f\n", k,index,i,a,ld2->valid[k],ld2->readings[k]);*/
	}
	
	ld2->min_theta = ld2->theta[0];
	ld2->max_theta = ld2->theta[n-1];
	ld2->tv = ld->tv;

	copy_d(ld->odometry, 3, ld2->odometry);
	copy_d(ld->estimate, 3, ld2->estimate);
	
	
	return ld2;
}

