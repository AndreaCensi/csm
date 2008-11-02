#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>

#include <math.h>
#include <options/options.h>

#include "../csm/csm_all.h"

struct ld_noise_params {
	int seed;

	double sigma_xy;
	double sigma_theta_deg;
	
	const char* file_input;
	const char* file_output;
	
	int debug;
};

const char * banner = 
	"A simple program for adding slip to odometry \n\n"
		"The 'odometry' field is set to 'true_pose' + noise.\n"
		"If 'true_pose' is not available, then the 'odometry' \n"
		"field is set to 'odometry' + noise.\n\n"
		"Note: this program does *not* simulate the effect of \n"
		"slip or odometry error in a realistic way; each scan \n"
		"in the file is considered separately, the error does \n"
		"not depend on the traveled distance, etc.\n\n";

int main(int argc, const char ** argv) {
	sm_set_program_name(argv[0]);
	
	struct ld_noise_params p;
	
	options_banner(banner);
	
	struct option* ops = options_allocate(10);
	options_double(ops, "sigma_theta_deg", &p.sigma_theta_deg, 0.0, 
		"Std deviation of gaussian noise for theta (deg) (disabled if 0)");
	options_double(ops, "sigma_xy", &p.sigma_xy, 0.0, 
		"Std deviation of gaussian noise for x,y (disabled if 0)");
	options_int(ops, "seed", &p.seed, 0, 
		"Seed for random number generator (if 0, use GSL_RNG_SEED env. variable).");
	options_string(ops, "in", &p.file_input, "stdin", "Input file ");
	options_string(ops, "out", &p.file_output, "stdout", "Output file ");

	options_int(ops, "debug", &p.debug, 0, "Shows debug information");
	
		
	if(!options_parse_args(ops, argc, argv)) {
		options_print_help(ops, stderr);
		return -1;
	}
	
	sm_debug_write(p.debug);

	gsl_rng_env_setup();
	gsl_rng * rng = gsl_rng_alloc (gsl_rng_ranlxs0);
	if(p.seed != 0)
	gsl_rng_set(rng, (unsigned int) p.seed);
	
	
	FILE * in = open_file_for_reading(p.file_input);
	if(!in) return -3;

	FILE * out = open_file_for_writing(p.file_output);
	if(!out) return -2;

	LDP ld; int count=0;
	while( (ld = ld_read_smart(in))) {
		count++;
		if(!ld_valid_fields(ld))  {
			sm_error("Invalid laser data (#%d in file)\n", count);
			continue;
		}
		
		if(!any_nan(ld->true_pose, 3))
			copy_d( (const double*) ld->true_pose, 3, ld->odometry);
		
		double e[3] = {0,0,0};
		
		if(p.sigma_xy > 0) {
			e[0]  = gsl_ran_gaussian(rng, p.sigma_xy);
			e[1]  = gsl_ran_gaussian(rng, p.sigma_xy);
		}

		if(p.sigma_theta_deg > 0) {
			e[2] = gsl_ran_gaussian(rng, deg2rad(p.sigma_theta_deg));
		}
		
		ld->odometry[0] += e[0];
		ld->odometry[1] += e[1];
		ld->odometry[2] += e[2];
	
		sm_debug("Adding noise %s.\n", friendly_pose(e));
		
		ld_write_as_json(ld, out);
		
		ld_free(ld);
	}
	
	return 0;
}
