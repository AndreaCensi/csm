#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <math.h>
#include <options/options.h>
#include "../src/laser_data_json.h"

struct ld_noise_params {
	int seed;
	double discretization;
	double sigma;
};

int main(int argc, const char * argv[]) {
	struct ld_noise_params p;
	
	struct option* ops = options_allocate(3);
	options_double(ops, "discretization", &p.discretization, 0.0, 
		"Size of discretization (disabled if 0)");
	options_double(ops, "sigma", &p.sigma, 0.0, 
		"Std deviation of gaussian noise (disabled if 0)");
	options_int(ops, "seed", &p.seed, 0, 
		"Seed for random number generator (if 0, use GSL_RNG_SEED env. variable).");
		
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "A simple program for adding noise to sensor scans.\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	gsl_rng_env_setup();
	gsl_rng * rng = gsl_rng_alloc (gsl_rng_ranlxs0);
	if(p.seed != 0)
	gsl_rng_set(rng, (unsigned int) p.seed);

	LDP ld;
	while( (ld = ld_from_json_stream(stdin))) {
		int i;
		for(i=0;i<ld->nrays;i++) {
			if(!ld_valid_ray(ld, i)) continue;
			
			double * reading = ld->readings + i;
			if(p.sigma > 0)
				*reading += gsl_ran_gaussian(rng, p.sigma);
			if(p.discretization > 0)
				*reading -= fmod(*reading , p.discretization);
		}
		JO jo = ld_to_json(ld);
		puts(json_object_to_json_string(jo));
		puts("\n");
		jo_free(jo);
		ld_free(ld);
	}
	
	return 0;
}
