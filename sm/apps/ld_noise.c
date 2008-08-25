#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <math.h>
#include <libgen.h>

#include <options/options.h>
#include <csm/csm_all.h>

struct ld_noise_params {
	int seed;
	double discretization;
	double sigma;
	const char* file_input;
	const char* file_output;
};

int main(int argc, const char * argv[]) {
	sm_set_program_name(argv[0]);
	
	struct ld_noise_params p;
	
	struct option* ops = options_allocate(3);
	options_double(ops, "discretization", &p.discretization, 0.0, 
		"Size of discretization (disabled if 0)");
	options_double(ops, "sigma", &p.sigma, 0.0, 
		"Std deviation of gaussian noise (disabled if 0)");
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
			if(p.sigma > 0)
				*reading += gsl_ran_gaussian(rng, p.sigma);
			if(p.discretization > 0)
				*reading -= fmod(*reading , p.discretization);
		}
	
		ld_write_as_json(ld, out);
		ld_free(ld);
	}
	
	return 0;
}
