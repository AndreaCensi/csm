#include <string.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_math.h>
#include <math.h>
#include <options/options.h>

#include "../csm/csm_all.h"

struct ld_exp_tro1_params {
	int seed;

	double max_xy_error;
	double max_theta_error_deg;
	
	const char* file_input;
	const char* file_output1;
	const char* file_output2;
	
	int num_per_scan;
	
	int debug;
};

const char * banner = 
	"This program prepares the data for one of the experiments. \n\n"
		"The input is any sensor log (Carmen or JSON format) \n"
		"The output are two files that contain laser_ref and laser_sens\n"
		"(you have to match the i-th scan in the first file with the i-th\n"
		" in the second).\n\n"
		"The two files contain exactly the same date but for the 'odometry' field\n"
		"The odometry error is uniform in the intervals given.\n";

int main(int argc, const char ** argv) {
	sm_set_program_name(argv[0]);
	
	struct ld_exp_tro1_params p;
	
	options_banner(banner);
	
	struct option* ops = options_allocate(10);
	options_double(ops, "max_xy_error", &p.max_xy_error, 10.0, "Maximum error for x,y (m)");
	options_double(ops, "max_theta_error_deg", &p.max_theta_error_deg, 10.0, "Maximum error for orientation (deg)");
	options_int   (ops, "seed", &p.seed, 0, "Seed for random number generator (if 0, use GSL_RNG_SEED env. variable).");

	options_int(ops, "num_per_scan", &p.num_per_scan, 10, "Number of trials for each scan.");

	options_string(ops, "in", &p.file_input, "stdin", "Input file ");
	options_string(ops, "out1", &p.file_output1, "stdout", "Output file for first scan");
	options_string(ops, "out2", &p.file_output2, "stdout", "Output file for second scan");
	
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
	
	/* Open the two output files (possibly the same one) */
	
	FILE * in = open_file_for_reading(p.file_input);
	if(!in) return -3;

	FILE * out1 = open_file_for_writing(p.file_output1);
	if(!out1) return -2;
	
	FILE * out2;
	if(!strcmp(p.file_output1, p.file_output2)) {
		out1 = out2;
	} else {
		out2 = open_file_for_writing(p.file_output2);
		if(!out2) return -2;
	}

	/* Read laser data from input file */
	LDP ld; int count=0;
	while( (ld = ld_read_smart(in))) {
		count++;
		if(!ld_valid_fields(ld))  {
			sm_error("Invalid laser data (#%d in file)\n", count);
			continue;
		}
		
		for(int n=0; n < p.num_per_scan; n++) {					
			ld->true_pose[0] = 0;
			ld->true_pose[1] = 0;
			ld->true_pose[2] = 0;
			
			ld->odometry[0] = 0;
			ld->odometry[1] = 0;
			ld->odometry[2] = 0;
			
			ld_write_as_json(ld, out1);

			ld->odometry[0] = 2*(gsl_rng_uniform(rng)-0.5) * p.max_xy_error;
			ld->odometry[1] = 2*(gsl_rng_uniform(rng)-0.5) * p.max_xy_error;
			ld->odometry[2] = 2*(gsl_rng_uniform(rng)-0.5) * deg2rad(p.max_theta_error_deg);
			
			ld_write_as_json(ld, out2);
		}

		ld_free(ld);
	}
	
	return 0;
}
