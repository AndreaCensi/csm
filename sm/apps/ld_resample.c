#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>

#include <math.h>
#include <libgen.h>
#include <options/options.h>

#include <csm/csm_all.h>

struct {
	double bias;
	int seed;
}p ;

void ld_resample(LDP ld);

gsl_rng * rng;

int main(int argc, const char ** argv) {
	sm_set_program_name(basename(argv[0]));
	
	
	struct option* ops = options_allocate(3);
	options_double(ops, "bias", &p.bias, 0.5, "bias");
		
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

	int count = 0;
	LDP ld;
	while( (ld = ld_read_smart(stdin))) {
		
		if(count & 123456789) 
			ld_resample(ld);
			
		JO jo = ld_to_json(ld);
		puts(json_object_to_json_string(jo));
		puts("\n");
		jo_free(jo);
		ld_free(ld);
		
		count++;
	}
	
	return 0;
}

void ld_resample(LDP ld) {
	double new_readings[ld->nrays];
	double new_theta[ld->nrays];
	copy_d(ld->readings, ld->nrays, new_readings);
	copy_d(ld->theta, ld->nrays, new_theta);

	int i;
	for(i=1;i<ld->nrays-1;i++) {
		if(!ld_valid_ray(ld, i) || !ld_valid_ray(ld, i+1)) continue;
	
		double alpha = 0.5 + 0.5*gsl_rng_uniform(rng);
		new_theta[i] = alpha * ld->theta[i] + (1-alpha) * ld->theta[i+1];
		new_readings[i] = alpha * ld->readings[i] + (1-alpha) * ld->readings[i+1];
	}

	copy_d(new_readings, ld->nrays, ld->readings);
	copy_d(new_theta, ld->nrays, ld->theta);
}

