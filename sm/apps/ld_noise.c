#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <math.h>
#include <libgen.h>

#include <options/options.h>
#include "../csm/csm_all.h"

struct ld_noise_params {
	int seed;
	double discretization;
	double sigma;
	const char* file_input;
	const char* file_output;
	int lambertian;
};

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
	options_banner("ld_noise: Adds noise to readings in a scan");
	
	struct ld_noise_params p;
	
	struct option* ops = options_allocate(20);
	options_double(ops, "discretization", &p.discretization, 0.0, 
		"Size of discretization (disabled if 0)");
	options_double(ops, "sigma", &p.sigma, 0.0, 
		"Std deviation of gaussian noise (disabled if 0)");
	options_int(ops, "lambertian", &p.lambertian, 0, 
		"Use lambertian model cov = sigma^2 / cos(beta^2) where beta is the incidence. Need have alpha or true_alpha.");
	options_int(ops, "seed", &p.seed, 0, 
		"Seed for random number generator (if 0, use GSL_RNG_SEED env. variable).");
	options_string(ops, "in", &p.file_input, "stdin", "Input file ");
	options_string(ops, "out", &p.file_output, "stdout", "Output file ");
		
		
	if(!options_parse_args(ops, argc, argv)) {
		fprintf(stderr, "A simple program for adding noise to sensor scans.\n\nUsage:\n");
		options_print_help(ops, stderr);
		return -1;
	}

	FILE * in = open_file_for_reading(p.file_input);
	if(!in) return -3;

	FILE * out = open_file_for_writing(p.file_output);
	if(!out) return -2;


	gsl_rng_env_setup();
	gsl_rng * rng = gsl_rng_alloc (gsl_rng_ranlxs0);
	if(p.seed != 0)
	gsl_rng_set(rng, (unsigned int) p.seed);

	LDP ld; int count = 0;
	while( (ld = ld_from_json_stream(in))) {
		if(!ld_valid_fields(ld))  {
			sm_error("Invalid laser data (#%d in file)\n", count);
			continue;
		}
		
		int i;
		for(i=0;i<ld->nrays;i++) {
			if(!ld->valid[i]) continue;
			
			double * reading = ld->readings + i;
			if(p.sigma > 0) {
				double add_sigma = p.sigma;
				
				if(p.lambertian) {

					int have_alpha = 0;
					double alpha = 0;
					if(!is_nan(ld->true_alpha[i])) {
						alpha = ld->true_alpha[i];
						have_alpha = 1;
					} else if(ld->alpha_valid[i]) {
						alpha = ld->alpha[i];;
						have_alpha = 1;
					} else have_alpha = 0;

					if(have_alpha) {
						/* Recall that alpha points outside the surface */
						double beta = (alpha+M_PI) - ld->theta[i];
					    add_sigma = p.sigma / cos(beta);
					} else {
						sm_error("Because lambertian is active, I need either true_alpha[] or alpha[]");
						ld_write_as_json(ld, stderr);
						return -1;
					}
					
				} 
				
			   *reading += gsl_ran_gaussian(rng, add_sigma);
				
				if(is_nan(ld->readings_sigma[i])) {
					ld->readings_sigma[i] = add_sigma;
				} else {
					ld->readings_sigma[i] = sqrt(square(add_sigma) + square(ld->readings_sigma[i]));
				}
			}
			if(p.discretization > 0)
				*reading -= fmod(*reading , p.discretization);
		}
	
		ld_write_as_json(ld, out);
		ld_free(ld);
	}
	
	return 0;
}
