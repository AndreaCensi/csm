#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>

#include <math.h>
#include <libgen.h>
#include <options/options.h>

#include <csm/csm_all.h>

struct {
	double interval;
	int seed;
}p ;

LDP ld_resample(LDP ld);

gsl_rng * rng;

int main(int argc, const char ** argv) {
	sm_set_program_name(basename(argv[0]));
	
	
	struct option* ops = options_allocate(3);
	options_double(ops, "interval", &p.interval, sqrt(2.0), " 1 = no resampling");
		
	options_int(ops, "seed", &p.seed, 0, 
		"Seed for random number generator (if 0, use GSL_RNG_SEED env. variable).");
		
	if(!options_parse_args(ops, argc, argv)) {
/*		fprintf(stderr, "A simple program for adding slip to odometry \n\n"
			"The 'odometry' field is set to 'true_pose' + noise.\n"
			"If 'true_pose' is not available, then the 'odometry' \n"
			"field is set to 'odometry' + noise.\n\n"
			"Note: this program does *not* simulate the effect of \n"
			"slip or odometry error in a realistic way; each scan \n"
			"in the file is considered separately, the error does \n"
			"not depend on the traveled distance, etc.\n\n");*/
		options_print_help(ops, stderr);
		return -1;
	}

	gsl_rng_env_setup();
	rng = gsl_rng_alloc (gsl_rng_ranlxs0);
	if(p.seed != 0)
	gsl_rng_set(rng, (unsigned int) p.seed);

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
	int n = (int) (floor(ld->nrays / p.interval)-1);
	
	LDP ld2 = ld_alloc_new(n);	
	int k;
	for(k=0;k<n;k++) {
		double index = k * p.interval;
		int i = (int) floor(index);
		double a = 1 - (index - i);

		ld2->theta[k] = a * ld->theta[i] + (1-a) * ld->theta[i+1];

		if(!ld->valid[i] || !ld->valid[i+1]) {
			ld2->valid[k] = 0;
			ld2->readings[k] = NAN;
		} else {
			ld2->readings[k] = a * ld->readings[i] + (1-a) * ld->readings[i+1];
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

